#pragma once

#include "ECS.h"

namespace ECS
{

using MaskResult = unsigned char;

// Mask result flags, the situation can be combined.
enum MaskResultFlag
	: unsigned char
{ 
	InvalidEntityID			= 1,	// 0001
	Success					= 2,	// 0010 
	RedundancyComponent		= 4,	// 0100
	InvalidComponentType	= 8		// 1000
};

// the EntityFreeBlock is used to store the free entity ID,
// all the avaliable id are stored between the EntityFreBlock.start~end.
typedef struct EntityFreeBlock
{
public:
	EntityID			start;
	EntityID			end;
	EntityFreeBlock*	next;
}EntityFreeBlock, *PEntityFreeBlock;


// The iterator to traverse the desired entity.
template<typename ...COMPONENT_TYPES>
class EntityIter
{
private:
	// target entity componetMask
	const ComponentMask&		_desiredMask;
	// max entity Count.
	const size_t&				_maxEntityCount;
	// the pointer to the EntityManager's maskPool
	const ComponentMask* const	_maskPool;
	// the freeList which contain the invalid EntityIDs.
	PEntityFreeBlock			_internalHead;
	PEntityFreeBlock			_internalTail;
	// next Entity
	EntityID					_currID;

public:
	EntityIter(
		const ComponentMask&		desiredMask,
		const size_t&				maxEntityCount,
		const ComponentMask* const	maskPool,
		PEntityFreeBlock			internalHead,
		PEntityFreeBlock			internalTail,
		EntityID					startID = 0);

	EntityID operator*();

	EntityIter& operator ++();

	bool operator != (const EntityIter<COMPONENT_TYPES...>& other) const;

private:
	EntityID getInternalStart();
	EntityID getInternalEnd();
};// class EntityIter

template<typename ...COMPONENT_TYPES>
class EntityRange
{
private:
	// target entity componetMask
	const ComponentMask				_desiredMask;
	// max entity Count.
	const size_t					_maxEntityCount;
	// the pointer to the EntityManager's maskPool
	const ComponentMask* const		_maskPool;
	// the freeList which contain the invalid EntityIDs.
	PEntityFreeBlock				_pFreeList;

public:
	EntityRange(
		const size_t&				maxEntityCount, 
		const ComponentMask* const	maskPool,
		PEntityFreeBlock			pFreeList);

	EntityIter<COMPONENT_TYPES...> begin();
	
	EntityIter<COMPONENT_TYPES...> end();
};


template<typename ...COMPONENT_TYPES>
inline EntityIter<COMPONENT_TYPES...>::EntityIter(
	const ComponentMask&				desiredMask,
	const size_t &						maxEntityCount,
	const ComponentMask * const			pMaskPool,
	PEntityFreeBlock					internalHead,
	PEntityFreeBlock					internalTail,
	EntityID							startID)
	:
	_desiredMask(desiredMask),
	_maxEntityCount(maxEntityCount),
	_maskPool(pMaskPool),
	_internalHead(internalHead),
	_internalTail(internalTail),
	_currID(startID)
{
}

template<typename ...COMPONENT_TYPES>
inline EntityID EntityIter<COMPONENT_TYPES...>::operator*()
{
	return _currID;
}
template<typename ...COMPONENT_TYPES>
inline EntityIter<COMPONENT_TYPES...> & EntityIter<COMPONENT_TYPES...>::operator++()
{
	static EntityID internalStart = _currID;
	static EntityID internalEnd = getInternalEnd();
	static bool findAnotherOne;

	findAnotherOne = false;
	internalStart = _currID;

	while (true)
	{
		// _currID has been accessed,
		// so the first thing is add one to it.;
		++_currID;

		// loop the _currID to internalEnd until we find a id that have the desiredMask.
		for (; _currID <= internalEnd; ++_currID)
		{
			if ((_maskPool[_currID] & _desiredMask) == _desiredMask)
			{
				findAnotherOne = true;
				break;
			}
		}

		// have we find a avaliable id, or have we walked through the whole _maskPool
		if (findAnotherOne || _internalTail == nullptr)
		{
			break;
		}
		else
		{
			// Found no avaliable id, move the internal to the next.
			_internalHead = _internalTail;
			_internalTail = _internalTail->next;

			// update the internalStart and internalEnd,
			// continue while loop.
			internalStart	= getInternalStart();
			internalEnd		= getInternalEnd();
			_currID = internalStart;
		}
	}

	// if the _currID greater than the _maxEntityCount,
	// it means no avaliable id was found this time, 
	// return 0, and the 'range for loop' should be stopped when it return 0.
	_currID = _currID > _maxEntityCount ? 0 : _currID;
	
	return *this;
}

template<typename ...COMPONENT_TYPES>
inline bool EntityIter<COMPONENT_TYPES...>::operator!=(const EntityIter<COMPONENT_TYPES...> & end) const
{
	return _currID != 0;
}

template<typename ...COMPONENT_TYPES>
inline EntityID EntityIter<COMPONENT_TYPES...>::getInternalStart()
{
	return _internalHead == nullptr ? 1 : _internalHead->end + 1;
}

template<typename ...COMPONENT_TYPES>
inline EntityID EntityIter<COMPONENT_TYPES...>::getInternalEnd()
{
	return _internalTail == nullptr ? _maxEntityCount : _internalTail->start - 1;
}

template<typename ...COMPONENT_TYPES>
inline 
EntityRange<COMPONENT_TYPES...>::EntityRange(
	const size_t&				maxEntityCount,
	const ComponentMask* const	maskPool,
	PEntityFreeBlock			pFreeList)
	:
	_desiredMask(getComponentMask<COMPONENT_TYPES...>()),
	_maxEntityCount(maxEntityCount),
	_maskPool(maskPool),
	_pFreeList(pFreeList)
{
}
template<typename ...COMPONENT_TYPES>
inline EntityIter<COMPONENT_TYPES...> EntityRange<COMPONENT_TYPES...>::begin()
{
	// try to get the first desiredID
	EntityID firstId;
	EntityID internalStart, internalEnd;

	PEntityFreeBlock internalHead, internalTail;

	// next two pointer point to the two block inside which is the valid entities.
	internalHead = (_pFreeList && _pFreeList->start == 1) ? _pFreeList : nullptr;
	internalTail = (internalHead) ? _pFreeList->next : _pFreeList;

	// get the start valid entityID
	auto getInternalStart = [&internalHead]() ->EntityID {
		return internalHead == nullptr ? 1 : internalHead->end + 1;
	};

	// get the end valid entityID
	auto getInternalEnd = [&internalTail, this]() {
		return internalTail == nullptr ? _maxEntityCount : internalTail->start - 1;
	};

	bool findFirstID = false;

	while (true)
	{
		internalStart = getInternalStart();
		internalEnd = getInternalEnd();

		for (firstId = internalStart; firstId <= internalEnd; ++firstId)
		{
			if ((_maskPool[firstId] & _desiredMask) == _desiredMask)
			{
				// a flag to break the outer while loop.
				findFirstID = true;
				break;
			}
		}

		if (findFirstID || internalTail == nullptr)
		{
			break;
		}
		else
		{
			internalHead = internalTail;
			internalTail = internalTail->next;
		}
	}

	// have we find through all the entities?
	// If we did, it means that we don't find even one entity 
	// which meet our requirement, set it to zero.
	firstId = firstId > _maxEntityCount ? 0 : firstId;
	
	return EntityIter<COMPONENT_TYPES...>(_desiredMask, _maxEntityCount, _maskPool, internalHead, internalTail, firstId);
}
template<typename ...COMPONENT_TYPES>
inline EntityIter<COMPONENT_TYPES...> EntityRange<COMPONENT_TYPES...>::end()
{
	// This end() return a iterator which has no use,
	// because all the functionality is implement in the iterator of the begin().
	return EntityIter<COMPONENT_TYPES...>(_desiredMask, _maxEntityCount, _maskPool, nullptr, nullptr, 0);
}
}// namespace ECS
