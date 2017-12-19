#pragma once
#include "ECS.h"
#include <MyTools\TypeTool.h>
#include <unordered_map>
#include "BaseComponent.h"

namespace ECS
{

class BaseComponentSecretDesigner
{
protected:
	// set the _hostID of the component.
	inline static void setComponentEntityID(BaseComponent * pcmp, EntityID newID)
	{
		pcmp->_hostID = newID;
	}
};


template<typename COMPONENT_TYPE>
class ComponentManager
	:public BaseComponentSecretDesigner // with this class the ComponentManager can change the Component's EntityID,
	// and hide it from outside the ComponentManager.
{
public:
	ComponentManager(size_t maxSize);
	~ComponentManager();

	// generate  a new component with the entityID,
	// pass all the rest argument to the component's constructor.
	template<typename ...CONSTRUCT_ARGS>
	COMPONENT_TYPE *	newComponnet(EntityID entityID, CONSTRUCT_ARGS&&...args);
	// Get Component pointer through entityID, if not exist, return nullptr.
	COMPONENT_TYPE *	getComponent(EntityID entityID);
	// Get Component pointer through entityID, if not exist, return nullptr.
	COMPONENT_TYPE *	operator [](EntityID entityID);
	// delete the component use the EntityID.
	bool				removeComponent(EntityID removedID);
	size_t				getMaxSize();
	size_t				getUsedCount();

private:
	// the max avaliable component count
	size_t _maxSize;
	size_t _usedComponentCount;
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
inline COMPONENT_TYPE * 
ComponentManager<COMPONENT_TYPE>::getComponent
(EntityID entityID)
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
inline COMPONENT_TYPE * ComponentManager<COMPONENT_TYPE>::operator[](EntityID entityID)
{
	return this->getComponent(entityID);
}

template<typename COMPONENT_TYPE>
inline bool ComponentManager<COMPONENT_TYPE>::removeComponent(EntityID removedID)
{
	auto iterCmp = _lookUpTable.find(removedID);
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
		--_usedComponentCount;
		return true;
	}
}

template<typename COMPONENT_TYPE>
inline size_t ComponentManager<COMPONENT_TYPE>::getMaxSize()
{
	return _maxSize;
}

template<typename COMPONENT_TYPE>
inline size_t ComponentManager<COMPONENT_TYPE>::getUsedCount()
{
	return _usedComponentCount;
}

template<typename COMPONENT_TYPE>
template<typename ...CONSTRUCT_ARGS>
inline COMPONENT_TYPE * 
ComponentManager<COMPONENT_TYPE>::
newComponnet
(EntityID entityID, CONSTRUCT_ARGS&&...args)
{
	// prevent from create the component with the same entityID.
	ASSERT(nullptr == this->getComponent(entityID));
	if (_usedComponentCount == _maxSize)
	{
		return nullptr;
	}
	auto * pNewComp = new COMPONENT_TYPE(std::forward<CONSTRUCT_ARGS>(args)...);
	setComponentEntityID(pNewComp, entityID);
	_lookUpTable[entityID] = pNewComp;
	++_usedComponentCount;
	return pNewComp;
}

}// namespace ECS

