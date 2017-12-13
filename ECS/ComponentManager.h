#pragma once
#include "ECS.h"
#include <MyTools\TypeTool.h>

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

	template<typename ...CONSTRUCT_ARGS>
	
	COMPONENT_TYPE *  newComponnet(EntityID entityID, CONSTRUCT_ARGS&&...args);
	//bool removeComponent(EntityID removedID);

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
template<typename ...CONSTRUCT_ARGS>
inline COMPONENT_TYPE * ComponentManager<COMPONENT_TYPE>::newComponnet(EntityID entityID, CONSTRUCT_ARGS&&...args)
{
	auto * pNewComp = new COMPONENT_TYPE(std::forward<COMPONENT_TYPE>(args)...);
	_lookUpTable[EntityID] = pNewComp;
}

}// namespace ECS

