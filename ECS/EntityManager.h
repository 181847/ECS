#pragma once
#include "ECS.h"
#include <bitset>

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
	// the enum used to indicate the operationResult of maskComponentType,
	// this enum type is major used in the recursion, 
	// and we will combine each recursion result together.

	// be aware that the EntityID '0' means invalid Entity,
	static const size_t maxEntityCount = 2048;

	// the EntityFreeBlock is used to store the free entity ID,
	// all the avaliable id are stored between the EntityFreBlock.start~end.
	// which entityID can be used .
	typedef struct EntityFreeBlock
	{
	public:
		EntityID start;
		EntityID end;
		EntityFreeBlock* next;
	}EntityFreeBlock, *PEntityFreeBlock;

public:
	enum MaskResultFlag
		: unsigned char
	{ 
		InvalidEntityID			= 1,	// 0001
		Success					= 2,	// 0010 
		RedundancyComponent		= 4,	// 0100
		InvalidComponentType	= 8		// 1000
	};
	using MaskResult = unsigned char;

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
inline EntityManager::MaskResult EntityManager::maskSingleComponentType(EntityID entityID)
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
inline EntityManager::MaskResult EntityManager::maskComponentType(EntityID entityID)
{
	MaskResult result = 0;
	unsigned char zeros[] = { (result |= maskSingleComponentType<COMPONENT_TYPES>(entityID), 0)... };
	return result;
}

}// namespace ECS


