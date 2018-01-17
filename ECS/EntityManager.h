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
// Traits:
//		const MaxEntityCount;
template<typename Traits>
class EntityManager
{
public:
	// singleton
	static EntityManager<Traits>* getInstance();

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
	bool		haveComponent(EntityID entityID);
	
	// get the iterator of the entity which should have the specific ComponentType.
	template<typename ...COMPONENT_TYPES>
	auto		RangeEntities();
	
	size_t getSize() const;
	size_t getUsedIDCount() const;
private:
	EntityManager();
	~EntityManager();

	// add another one for entityID 0,
	// which means a invalid ID.
	ComponentMask		_maskPool[1 + Traits::MaxEntityCount];
	PEntityFreeBlock	_freeList;
	size_t				_usedID;
};

template<typename Traits>
template<typename COMPONENT_TYPE>
inline MaskResult EntityManager<Traits>::maskSingleComponentType(EntityID entityID)
{
	ASSERT(isValid(entityID));
#ifdef _DEBUG
	// if in the debug mode.
	// use a static var to store the componentTypeID.
	static const ComponentTypeID idForTheComponentType =
		ComponentIDGenerator::getID<COMPONENT_TYPE>();
	// the id must greater than 0;
	ASSERT(idForTheComponentType > 0 && "no ID found with the ComponentType, please ensure you call newID() with the ComponentType first.");

	// always use a static var to store the mask for the component type.
	// 1ull means 'number one whose type is unsigned long long'
	static const ComponentMask maskForTheComponentType{1ull << idForTheComponentType };
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

template<typename Traits>
template<typename ...COMPONENT_TYPES>
inline MaskResult EntityManager<Traits>::maskComponentType(EntityID entityID)
{
	ASSERT(isValid(entityID));
#ifdef _DEBUG
	
	// Using the initialization of the array to 
	// check each component type is registered before this function called.
	// Make it 'static const' so the check only have to do once.
	static const ComponentTypeID cmpIDArr[] = 
	{ 
		// place holder for the array.
		ComponentTypeID(0),

		(
			// for each component type, 
			// get its id and check if it > 0,
			// if not, it means the type haven't been registered until now.
			// Please ensure you call ComponentIDGenerator::newID<COMPONENT_TYPES>() before this function is called.
			ASSERT(ComponentIDGenerator::getID<COMPONENT_TYPES>() != 0 
				&& "no ID found with the ComponentType")
			,

			// place holder for the array.
			ComponentTypeID(0)
		)... // Expend variadic templates parameters.
	};
#endif

	// Pre store the mask of the specific combination of the multiple component types.
	static const ComponentMask combinedMask = ECS::getComponentMask<COMPONENT_TYPES...>();

	// In the case that there is no ComponentTypes passed in,
	// the combinedMask shouble be like "0000001" which means the last bit moved zero step,
	// and there is not any componet type information in it.
	// Just return success.
	if (combinedMask == 1)
	{
		return MaskResultFlag::Success;
	}

	MaskResult result = 0;
	// does the entity already have some of the componentType?
	if ((_maskPool[entityID] & combinedMask).any())
	{
		result |= MaskResultFlag::RedundancyComponent;
	}

	_maskPool[entityID] |= combinedMask;
	result |= MaskResultFlag::Success;

	return result;
}

template<typename Traits>
template<typename ...COMPONENT_TYPES>
inline bool EntityManager<Traits>::haveComponent(EntityID entityID)
{
	ASSERT(isValid(entityID));
	const ComponentMask combinedMask = ECS::getComponentMask<COMPONENT_TYPES...>();

	if ((_maskPool[entityID] & combinedMask) == combinedMask)
	{
		return true;
	}
	else
	{
		return false;
	}
}

template<typename Traits>
template<typename ...COMPONENT_TYPES>
inline auto EntityManager<Traits>::RangeEntities()
{
	return EntityRange<COMPONENT_TYPES...>(Traits::MaxEntityCount, _maskPool, _freeList);
}

template<typename Traits>
EntityManager<Traits> * EntityManager<Traits>::getInstance()
{
	static EntityManager<Traits> singletonEntityManager;
	return &singletonEntityManager;
}

template<typename Traits>
EntityID EntityManager<Traits>::newEntity()
{
	if (_freeList == nullptr)
	{
		// no invalid ID.
		return 0;
	}

	EntityID retID = 0;

	// if there is only one id left in the first block.
	if (_freeList->start == _freeList->end)
	{
		retID = _freeList->start;
		// delete the first block
		// move the pointer to the next block.
		PEntityFreeBlock deleteBlock = _freeList;
		_freeList = _freeList->next;
		delete deleteBlock;
	}
	else
	{
		// the first block have more than one entity id,
		// return the start, and substract it by one.
		retID = _freeList->start++;
	}

	++_usedID;
	// clear the mask.
	_maskPool[retID].reset();
	return retID;
}

template<typename Traits>
bool EntityManager<Traits>::destoryEntity(EntityID destoriedID)
{
	// the entityID shouldn't be zero.
	ASSERT(destoriedID != 0);

	PEntityFreeBlock prev(nullptr), curr(_freeList);
	PEntityFreeBlock newBlock = nullptr;

	while (curr)
	{
		// break the loop if the curr block is start
		// after the destoriedID.
		if (curr->start > destoriedID)
		{
			break;
		}
		else
		{
			prev = curr;
			curr = curr->next;
		}
	}

	unsigned char operationFlag = 0;
	if (prev == nullptr)
	{
		operationFlag |= 1;
	}
	else if (prev->end >= destoriedID)
	{
		// this is a error,
		// it means that the caller is trying to destory the id
		// which is still in the freeBlock.
		// More detail can be found in the 'EntityManager::isValid(EntityID)'
		return false;
	}
	else if ((prev->end + 1) == destoriedID)
	{
		operationFlag |= 2;
	}

	if (curr == nullptr)
	{
		operationFlag |= 4;
	}
	else if ((curr->start - 1) == destoriedID)
	{
		operationFlag |= 8;
	}


	// comment notice
	// /\:				nullptr
	// -|-|:			the pointer point to some block
	// pointerA \_/ destoriedID:  
	//					there is a gap between pointerA's block and the destoriedID
	// pointerA -> destoriedID:  
	//					the pointerA's block concatenate the destoriedID
	switch (operationFlag)
	{
		// prev /\  curr /\, empty list
	case (1 | 4):
		// prev /\ , curr-|-|, prev\_/ dest\_/ curr
	case 1:
		_freeList = new EntityFreeBlock();
		_freeList->start = _freeList->end = destoriedID;
		_freeList->next = curr;
		break;

		// prev /\  curr-|-|, prev\_/ dest<-curr
	case (1 | 8):
		// prev-|-|, curr-|-|, prev\_/ dest<-curr
	case 8:
		// add destoreidID to the curr block.
		curr->start = destoriedID;
		break;

		// prev-|-|, curr /\, prev -> dest\_/ curr
	case (4 | 2):
		// prev-|-|, curr-|-|, prev -> dest\_/ curr
	case 2:
		prev->end = destoriedID;
		break;

		// prev-|-|, curr /\, prev\_/ dest\_/ curr
	case 4:
		// prev-|-|, curr-|-|, prev\_/ dest\_/ curr
	case 0:
		newBlock = new EntityFreeBlock();
		newBlock->start = newBlock->end = destoriedID;
		// concat the new block
		prev->next = newBlock;
		newBlock->next = curr;
		break;

		// prev-|-|, curr-|-|, prev->next<-curr
	case (2 | 8):
		prev->end = curr->end;
		prev->next = curr->next;
		delete curr;
		break;

	default:
		ASSERT(false && "Unexpected situation");
	}

	--_usedID;
	return true;
}

template<typename Traits>
bool EntityManager<Traits>::isValid(EntityID checkID)
{
	// zero is alway the invalid ID.
	if (!checkID)
	{
		return false;
	}
	PEntityFreeBlock prev(nullptr), curr(_freeList);

	while (curr)
	{
		// break the loop if the curr block is start
		// after the destoriedID.
		if (curr->start > checkID)
		{
			break;
		}
		else
		{
			prev = curr;
			curr = curr->next;
		}
	}

	// the error Image:
	//prev						curr
	// |						/ \
	// V					   /   \
	//|-----------|			null OR	|-----------|
	//|start   end|					|start   end|
	//|-----------|					|-----------|
	//	   /\   /|
	//	   |OR /
	//	   |  /
	//	checkID

	unsigned char operationFlag = 0;
	if (prev != nullptr && (prev->end) >= checkID)
	{
		// this is a error,
		// it means that the caller is trying to destory the id
		// which is still in the freeBlock.
		return false;
	}

	return true;
}

template<typename Traits>
size_t EntityManager<Traits>::getSize() const
{
	return Traits::MaxEntityCount;
}

template<typename Traits>
size_t EntityManager<Traits>::getUsedIDCount() const
{
	return _usedID;
}

template<typename Traits>
EntityManager<Traits>::EntityManager()
	: _usedID(0)
{
	_freeList = new EntityFreeBlock();
	_freeList->start = 1;
	_freeList->end = Traits::MaxEntityCount;
	_freeList->next = nullptr;

	// Check the MaxEntityCount is greater than 0.
	assert(_freeList->end > 0);
}

template<typename Traits>
EntityManager<Traits>::~EntityManager()
{
}

}// namespace ECS


