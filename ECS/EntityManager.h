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

	// singleton
	static EntityManager* getInstance();

	// get a new Entity and return its ID.
	EntityID	newEntity();

	// destroy the Entity.
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

	// add another one for entityID 0,
	// which means a invalid ID.
	ComponentMask		_maskPool[1 + maxEntityCount];
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
	// Or here can just make it stand for the invalid component mask
	// now just left it.
	_maskPool[entityID] |= maskForTheComponentType;

	return MaskResultFlag::Success;
}

template<typename ...COMPONENT_TYPES>
inline MaskResult EntityManager::maskComponentType(EntityID entityID)
{
	ASSERT(isValid(entityID));
#ifdef _DEBUG
	static const size_t cmpLength = sizeof...(COMPONENT_TYPES);
	// be aware there is always a zero in the front, in case the COMPONTNE_TYPES is zero which make the length of the array zero.
	static ComponentTypeID cmpIDArr[] = { (0), (ComponentIDGenerator::getID<COMPONENT_TYPES>())... };
	for (size_t i = 1; i <= cmpLength; ++i)
	{
		ASSERT(cmpIDArr[i] != 0 && "no ID found with the ComponentType, please ensure you call newID() with the ComponentType first.");
	}
#endif
	MaskResult result = 0;
	// in the case that there is no ComponentTypes passed in,
	// just return success.
	if (cmpLength == 0)
	{
		result = MaskResultFlag::Success;
		return result;
	}

	ComponentMask combinedMask = ECS::getComponentMask<COMPONENT_TYPES...>();

	// does the entity already have some of the componentType?
	if ((_maskPool[entityID] & combinedMask).any())
	{
		result |= MaskResultFlag::RedundancyComponent;
	}

	_maskPool[entityID] |= combinedMask;
	result |= MaskResultFlag::Success;

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
			(false), // in case that the COMPONENT_TYPES is zero.
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


