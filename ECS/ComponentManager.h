#pragma once
#include "ECS.h"
#include <MyTools\TypeTool.h>
#include <unordered_map>

namespace ECS
{

// this struct have no memeber, 
// its main goal is used to generat the unique number
// for each COMPONENT_TYPE.
struct ComponetTypeIDSeed{};
typedef TypeTool::IDGenerator<ComponetTypeIDSeed> ComponentIDGenerator;


template<typename COMPONENT_TYPE>
class ComponentManager
{
public:
	ComponentManager(size_t maxSize);
	~ComponentManager();

	// generate  a new component with the entityID,
	// return the pointer.
	// all the arguments will be pass to the COMPONENT_TYPE's Constructor.
	template<typename ...CONSTRUCT_ARGS>
	COMPONENT_TYPE *  newComponnet(EntityID entityID, CONSTRUCT_ARGS&&...args);
	// get the 
	COMPONENT_TYPE * getComponent(EntityID entityID);
	// delete the component use the EntityID.
	bool removeComponent(EntityID removedID);

private:
	size_t _maxSize;
	std::unordered_map<EntityID, COMPONENT_TYPE*> _lookUpTable;
};

template<typename COMPONENT_TYPE>
inline ComponentManager<COMPONENT_TYPE>::ComponentManager(size_t maxSize)
	:_maxSize(maxSize)
{
}

template<typename COMPONENT_TYPE>
inline ComponentManager<COMPONENT_TYPE>::~ComponentManager()
{
}

template<typename COMPONENT_TYPE>
inline COMPONENT_TYPE * ComponentManager<COMPONENT_TYPE>::getComponent(EntityID entityID)
{
	auto iterCmp = _lookUpTable.find(entityID);
	if (iterCmp == _lookUpTable.end())
	{
		return nullptr;
	}
	else
	{
		return iterCmp->second;
	}
}

template<typename COMPONENT_TYPE>
inline bool ComponentManager<COMPONENT_TYPE>::removeComponent(EntityID removedID)
{
	auto iterCmp = _lookUpTable.find(entityID);
	if (iterCmp == _lookUpTable.end())
	{
		return false;
	}
	else
	{
		// delete the pointer.
		delete iterCmp->second;
		// erase it from the map
		_lookUpTable.erase(iterCmp);
		return true;
	}
}

template<typename COMPONENT_TYPE>
template<typename ...CONSTRUCT_ARGS>
inline COMPONENT_TYPE * ComponentManager<COMPONENT_TYPE>::newComponnet(
	EntityID entityID, CONSTRUCT_ARGS&&...args)
{
	auto * pNewComp = new COMPONENT_TYPE(std::forward<CONSTRUCT_ARGS>(args)...);
	_lookUpTable[entityID] = pNewComp;
	return pNewComp;
}

}// namespace ECS

