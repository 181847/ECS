#pragma once

#include "ECS.h"

namespace ECS
{

using MaskResult = unsigned char;

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

// The iterator to traverse the desired entity.
template<typename ...COMPONENT_TYPES>
class EntityIter
{
private:
	// target entity componetMask
	const ComponentMask _desiredMask;
	// max entity Count.
	const size_t _maxEntityCount;
	// the pointer to the EntityManager's maskPool
	const ComponentMask* const _maskPool;
	// the freeList which contain the invalid EntityIDs.
	PEntityFreeBlock _pFreeList;
	// next Entity
	EntityID _nextID;

public:
	EntityIter(
		const size_t&				maxEntityCount,
		const ComponentMask* const	maskPool,
		PEntityFreeBlock			pFreeListHead);

	EntityID operator*();

	EntityIter& operator ++();

	bool operator != (const EntityIter<COMPONENT_TYPES...>& end) const;
};// class EntityIter



template<typename ...COMPONENT_TYPES>
inline EntityIter<COMPONENT_TYPES...>::EntityIter(
	const size_t & maxEntityCount,
	const ComponentMask * const pMaskPool,
	PEntityFreeBlock pFreeListHead)
	:
	_desiredMask(getComponentMask<COMPONENT_TYPES...>()),
	_maxEntityCount(maxEntityCount),
	_maskPool(pMaskPool),
	_pFreeList(pFreeListHead)
{

}
template<typename ...COMPONENT_TYPES>
inline EntityID EntityIter<COMPONENT_TYPES...>::operator*()
{
	return _nextID;
}
template<typename ...COMPONENT_TYPES>
inline EntityIter<COMPONENT_TYPES...> & EntityIter<COMPONENT_TYPES...>::operator++()
{

	return *this;
}

template<typename ...COMPONENT_TYPES>
inline bool EntityIter<COMPONENT_TYPES...>::operator!=(const EntityIter<COMPONENT_TYPES...> & end) const
{
	return _nextID != 0;
}

}// namespace ECS
