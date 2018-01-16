#include "EntityManager.h"

namespace ECS
{

//
//EntityManager * EntityManager::getInstance()
//{
//	static EntityManager singletonEntityManager;
//	return &singletonEntityManager;
//}
//
//EntityID EntityManager::newEntity()
//{
//	if (_freeList == nullptr)
//	{
//		// no invalid ID.
//		return 0;
//	}
//
//	EntityID retID = 0;
//
//	// if there is only one id left in the first block.
//	if (_freeList->start == _freeList->end)
//	{
//		retID = _freeList->start;
//		// delete the first block
//		// move the pointer to the next block.
//		PEntityFreeBlock deleteBlock = _freeList;
//		_freeList = _freeList->next;
//		delete deleteBlock;
//	}
//	else
//	{	
//		// the first block have more than one entity id,
//		// return the start, and substract it by one.
//		retID = _freeList->start++;
//	}
//
//	++_usedID;
//	// clear the mask.
//	_maskPool[retID].reset();
//	return retID;
//}
//
//bool EntityManager::destoryEntity(EntityID destoriedID)
//{
//	// the entityID shouldn't be zero.
//	ASSERT(destoriedID != 0);
//
//	PEntityFreeBlock prev(nullptr), curr(_freeList);
//	PEntityFreeBlock newBlock = nullptr;
//
//	while (curr)
//	{
//		// break the loop if the curr block is start
//		// after the destoriedID.
//		if (curr->start > destoriedID)
//		{
//			break;
//		}
//		else
//		{
//			prev = curr;
//			curr = curr->next;
//		}
//	}
//
//	unsigned char operationFlag = 0;
//	if (prev == nullptr)
//	{
//		operationFlag |= 1;
//	}
//	else if (prev->end >= destoriedID)
//	{
//		// this is a error,
//		// it means that the caller is trying to destory the id
//		// which is still in the freeBlock.
//		// More detail can be found in the 'EntityManager::isValid(EntityID)'
//		return false;
//	}
//	else if ((prev->end + 1) == destoriedID)
//	{
//		operationFlag |= 2;
//	}
//
//	if (curr == nullptr)
//	{
//		operationFlag |= 4;
//	}
//	else if ((curr->start - 1) == destoriedID)
//	{
//		operationFlag |= 8;
//	}
//
//
//	// comment notice
//	// /\:				nullptr
//	// -|-|:			the pointer point to some block
//	// pointerA \_/ destoriedID:  
//	//					there is a gap between pointerA's block and the destoriedID
//	// pointerA -> destoriedID:  
//	//					the pointerA's block concatenate the destoriedID
//	switch(operationFlag)
//	{
//		// prev /\  curr /\, empty list
//	case (1 | 4):
//		// prev /\ , curr-|-|, prev\_/ dest\_/ curr
//	case 1:
//		_freeList = new EntityFreeBlock();
//		_freeList->start = _freeList->end = destoriedID;
//		_freeList->next = curr;
//		break;
//
//		// prev /\  curr-|-|, prev\_/ dest<-curr
//	case (1 | 8):
//		// prev-|-|, curr-|-|, prev\_/ dest<-curr
//	case 8:
//		// add destoreidID to the curr block.
//		curr->start = destoriedID;
//		break;
//
//		// prev-|-|, curr /\, prev -> dest\_/ curr
//	case (4 | 2):
//		// prev-|-|, curr-|-|, prev -> dest\_/ curr
//	case 2:
//		prev->end = destoriedID;
//		break;
//
//		// prev-|-|, curr /\, prev\_/ dest\_/ curr
//	case 4:
//		// prev-|-|, curr-|-|, prev\_/ dest\_/ curr
//	case 0:
//		newBlock = new EntityFreeBlock();
//		newBlock->start = newBlock->end = destoriedID;
//		// concat the new block
//		prev->next = newBlock;
//		newBlock->next = curr;
//		break;
//
//		// prev-|-|, curr-|-|, prev->next<-curr
//	case (2 | 8):
//		prev->end = curr->end;
//		prev->next = curr->next;
//		delete curr;
//		break;
//
//	default:
//		ASSERT(false && "Unexpected situation");
//	}
//
//	--_usedID;
//	return true;
//}
//
//bool EntityManager::isValid(EntityID checkID)
//{
//	// zero is alway the invalid ID.
//	if ( ! checkID)
//	{
//		return false;
//	}
//	PEntityFreeBlock prev(nullptr), curr(_freeList);
//
//	while (curr)
//	{
//		// break the loop if the curr block is start
//		// after the destoriedID.
//		if (curr->start > checkID)
//		{
//			break;
//		}
//		else
//		{
//			prev = curr;
//			curr = curr->next;
//		}
//	}
//
//	// the error Image:
//	//prev						curr
//	// |						/ \
//	// V					   /   \
//	//|-----------|			null OR	|-----------|
//	//|start   end|					|start   end|
//	//|-----------|					|-----------|
//	//	   /\   /|
//	//	   |OR /
//	//	   |  /
//	//	checkID
//
//	unsigned char operationFlag = 0;
//	if (prev != nullptr && (prev->end) >= checkID)
//	{
//		// this is a error,
//		// it means that the caller is trying to destory the id
//		// which is still in the freeBlock.
//		return false;
//	}
//
//	return true;
//}
//
//size_t EntityManager::getSize() const
//{
//	// the zero is a invalid ID.
//	return maxEntityCount - 1;
//}
//
//size_t EntityManager::getUsedIDCount() const
//{
//	return _usedID;
//}
//
//EntityManager::EntityManager()
//	: _usedID(0)
//{
//	_freeList			= new EntityFreeBlock();
//	_freeList->start	= 1;
//	_freeList->end		= maxEntityCount;
//	_freeList->next		= nullptr;
//}
//
//
//EntityManager::~EntityManager()
//{
//}

}// namespace ECS

