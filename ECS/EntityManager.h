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
	enum MaskResult
		: unsigned char
	{ 
		InvalidEntityID			= 1,	// 0001
		Success					= 2,	// 0010 
		RedundancyComponent		= 4,	// 0100
		NoComponentType			= 8		// 1000
	};

	// be ware that the EntityID '0' means invalid Entity,
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
	// singlton
	static EntityManager* getInstance();
	// get a new Entity and return its ID.
	EntityID newEntity();
	// destory the Entity.
	bool destoryEntity(EntityID destoriedID);
	// check if the checkID is a valid id;
	bool isValid(EntityID checkID);

	// add all the COMPONENT_TYPE's type mask to the entityID's mask, 
	// this function can take multiple componentTypes and expand them
	// through the recursion.
	// recursion -> start
	template<typename FIRST_COMPONENT_TYPE, typename ...REST_COMPONENT_TYPES>
	MaskResult maskComponentType(EntityID entityID);
	// recursion <- end
	template<typename LAST_COMPONENT_TYPE>
	MaskResult maskComponentType(EntityID entityID);
	

	size_t getSize() const;
	size_t getUsedIDCount() const;
private:
	EntityManager();
	~EntityManager();

	ComponentMask _maskPool[maxEntityCount];
	PEntityFreeBlock _freeList;
	size_t _usedID;
};

}// namespace ECS


