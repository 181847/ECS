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

	using ComponentMask = std::bitset<32>;
	// be ware that the EntityID '0' means invalid Entity,
	static const size_t maxEntityCount = 1024;

	// the EntityFreeBlock is used to indicate 
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

	size_t getSize();

	size_t getUsedIDCount();
private:
	EntityManager();
	~EntityManager();

	ComponentMask _maskPool[maxEntityCount];
	PEntityFreeBlock _freeList;
	size_t _usedID;
};

}// namespace ECS


