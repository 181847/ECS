#pragma once
#include "ECS.h"
#include <bitset>
#include "EntityManagerTool.h"

namespace ECS
{

// the job of EntityManager is to generator the EntityID
// EntityID is just a number,
// inside the EntityManager, for each EntityID,
// it have a bitset to indicate which component type the Entity have,
// but the component is actually store in the ComponentManager,
// the EntityManager didn't store it.
// you can access the component through ComponentManager with the EntityID.
class EntityManager
{
public:
	// the enum used to indicate the operationResult of maskComponentType,
	// this enum type is major used in the recursion, 
	// and we will combine each recursion result together.

	// be aware that the EntityID '0' means invalid Entity,
	static const size_t maxEntityCount = 2048;

public:

	// singlton
	static EntityManager* getInstance();

	// get a new Entity and return its ID.
	EntityID	newEntity();

	// destory the Entity.
	bool		destoryEntity(EntityID destoriedID);

	// check if the checkID is a valid id;
	bool		isValid(EntityID checkID);

	// this function is major used in temaskComponentType<>(),
	// to mask only one type of Component.
	template<typename COMPONENT_TYPE>
	MaskResult	maskSingleComponentType(EntityID entityID);

	// to mask an entity component types with all the template args.
	template<typename ...COMPONENT_TYPES>
	MaskResult	maskComponentType(EntityID entityID);

	// check if the entity have the component type.
	template<typename ...COMPONENT_TYPES>
	bool haveComponent(EntityID entityID);
	
	// get the iterator of the entity which should have the specific ComponentType.
	template<typename ...COMPONENT_TYPES>
	auto RangeEntities();
	
	size_t getSize() const;
	size_t getUsedIDCount() const;
private:
	EntityManager();
	~EntityManager();

	ComponentMask		_maskPool[maxEntityCount];
	PEntityFreeBlock	_freeList;
	size_t				_usedID;
};



template<typename COMPONENT_TYPE>
inline MaskResult EntityManager::maskSingleComponentType(EntityID entityID)
{
	ASSERT(isValid(entityID));
#ifdef _DEBUG
	// if in the debug mode.
	// use a static var to store the componentTypeID.
	static ComponentTypeID idForTheComponentType =
		ComponentIDGenerator::getID<COMPONENT_TYPE>();
	// the id must greater than 0;
	ASSERT(idForTheComponentType > 0 && "no ID found with the ComponentType, please ensure you call newID() with the ComponentType first.");

	// always use a static var to store the mask for the component type.
	// 1ull means 'number one whose type is unsigned long long'
	static ComponentMask maskForTheComponentType{1ull << idForTheComponentType };
#else
	static ComponentMask maskForTheComponentType(1ull << ComponentIDGenerator::getID<LAST_COMPONENT_TYPE>());
#endif

	// does the entity already have the componentType?
	if ((_maskPool[entityID] & maskForTheComponentType).any())
	{
		return MaskResultFlag::RedundancyComponent;
	}

	//TODO: here waste one bit in the lower bitset, it can be improved in the future.
	_maskPool[entityID] |= maskForTheComponentType;

	return MaskResultFlag::Success;
}

template<typename ...COMPONENT_TYPES>
inline MaskResult EntityManager::maskComponentType(EntityID entityID)
{
	MaskResult result = 0;
	bool zeros[] = { (false), (result |= maskSingleComponentType<COMPONENT_TYPES>(entityID), false)... };
	return result;
}

template<typename ...COMPONENT_TYPES>
inline bool EntityManager::haveComponent(EntityID entityID)
{
	if (isValid(entityID))
	{
		ComponentMask mask(0);
		// get all the mask.
		bool args[] = { 
			(mask |= 1ull << 
			ComponentIDGenerator::getID<COMPONENT_TYPES>(), false)... };
		if ((_maskPool[entityID] & mask) == mask)
		{
			return true;
		}
	}
	return false;
}

template<typename ...COMPONENT_TYPES>
inline auto EntityManager::RangeEntities()
{
	return EntityRange<COMPONENT_TYPES...>(maxEntityCount, _maskPool, _freeList);
}

}// namespace ECS


