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
public:
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
		EntityManager::PEntityFreeBlock _pFreeList;
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
	bool zeros[] = { (result |= maskSingleComponentType<COMPONENT_TYPES>(entityID), false)... };
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
	return EntityIter<COMPONENT_TYPES...>(maxEntityCount, _maskPool, _freeList);
}

template<typename ...COMPONENT_TYPES>
inline EntityManager::EntityIter<COMPONENT_TYPES...>::EntityIter(
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
inline EntityID EntityManager::EntityIter<...COMPONENT_TYPES>::operator*()
{
	return _nextID;
}
template<typename ...COMPONENT_TYPES>
inline EntityManager::EntityIter<COMPONENT_TYPES...> & EntityManager::EntityIter<...COMPONENT_TYPES>::operator++()
{
	// TODO: 在此处插入 return 语句
	return *this;
}

template<typename ...COMPONENT_TYPES>
inline bool EntityManager::EntityIter<COMPONENT_TYPES...>::operator!=(const EntityManager::EntityIter<COMPONENT_TYPES...> & end) const
{
	return _nextID != 0;
}

}// namespace ECS


