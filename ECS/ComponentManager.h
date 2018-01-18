#pragma once
#include "ECS.h"
#include <MyTools\TypeTool.h>
#include <unordered_map>
#include "BaseComponent.h"

namespace ECS
{

// This class have the ability to change the hostID of a component,
// ComponentManager will extend it to get the ability.
class BaseComponentSecretDesigner
{
protected:
	// set the _hostID of the component.
	inline static void setComponentEntityID(BaseComponent * pcmp, EntityID newID)
	{
		pcmp->_hostID = newID;
	}
};

// The default ComponentManager Traits.
struct ComponentManagerDefaultTraits
{
	static const std::size_t MaxSize = 1024;
};

// Traits:
//		static size_t MaxSize; // the max component count.
template<typename COMPONENT_TYPE, typename Traits = ComponentManagerDefaultTraits>
class ComponentManager
	:public BaseComponentSecretDesigner 
	// With this supuer class, the ComponentManager can change the Component's EntityID,
	// and hide it's setter from outside the ComponentManager.
{
public:
	ComponentManager();
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
	bool					removeComponent(EntityID removedID);
	std::size_t				getMaxSize() const;
	std::size_t				getUsedCount() const;

private:
	std::size_t						_usedComponentCount;
	static const ComponentTypeID	_TypeID		= ComponentIDGenerator::IDOf<COMPONENT_TYPE>();
	std::unordered_map<EntityID, COMPONENT_TYPE*> _lookUpTable;
};

template<typename COMPONENT_TYPE, typename Traits>
inline ComponentManager<COMPONENT_TYPE, Traits>::ComponentManager()
{
}

template<typename COMPONENT_TYPE, typename Traits>
inline ComponentManager<COMPONENT_TYPE, Traits>::~ComponentManager()
{
}

template<typename COMPONENT_TYPE, typename Traits>
inline COMPONENT_TYPE * 
ComponentManager<COMPONENT_TYPE, Traits>::getComponent
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

template<typename COMPONENT_TYPE, typename Traits>
inline COMPONENT_TYPE * ComponentManager<COMPONENT_TYPE, Traits>::operator[](EntityID entityID)
{
	return this->getComponent(entityID);
}

template<typename COMPONENT_TYPE, typename Traits>
inline bool ComponentManager<COMPONENT_TYPE, Traits>::removeComponent(EntityID removedID)
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

template<typename COMPONENT_TYPE, typename Traits>
inline size_t ComponentManager<COMPONENT_TYPE, Traits>::getMaxSize() const
{
	return Traits::MaxSize;
}

template<typename COMPONENT_TYPE, typename Traits>
inline size_t ComponentManager<COMPONENT_TYPE, Traits>::getUsedCount() const
{
	return _usedComponentCount;
}

template<typename COMPONENT_TYPE, typename Traits>
template<typename ...CONSTRUCT_ARGS>
inline COMPONENT_TYPE * 
ComponentManager<COMPONENT_TYPE, Traits>::
newComponnet
(EntityID entityID, CONSTRUCT_ARGS&&...args)
{
	// Prevent from creating the component with the same entityID.
	assert(nullptr == this->getComponent(entityID));
	// Overflow ?
	assert(_usedComponentCount < Traits::MaxSize);

	auto * pNewComp = new COMPONENT_TYPE(std::forward<CONSTRUCT_ARGS>(args)...);
	setComponentEntityID(pNewComp, entityID);
	_lookUpTable[entityID] = pNewComp;
	++_usedComponentCount;

	return pNewComp;
}

}// namespace ECS

