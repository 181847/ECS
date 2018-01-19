#pragma once
#include "ECS.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include <functional>
#include <type_traits>
#include <MyTools\TypeTool.h>

namespace ECS
{

// The ECSWorld is used to store all the ComponentManager,
// and provide a function to get them,
// all the ComponentManager is using the template feature.
template<typename EntityManagerTraits, typename ...ComponentType>
class ECSWorld
{
	// Give the Implemented ECSWorld an alias.
	using ECSWorldAlias = ECSWorld<EntityManagerTraits, ComponentType...>;
public:
	// Create a new Entity, return its id.
	EntityID NewEntity();

	// Iterate all the entity that have all the component type in the template paramenters,
	// for each entity, get the id and the component it associated with, 
	// pass them to the std::function.
	template<typename ...COMPONENT_TYPE_LIST>
	void Foreach(std::function<void(EntityID, COMPONENT_TYPE_LIST*...)> theJob);

	// For only One entity, try to get its components whose type is listed in the template parameters,
	// if any type lacked, return false,
	// else pass the listed components to the std::function.
	template<typename ...COMPONENT_TYPE_LIST>
	bool ForOne(EntityID id, std::function<void(COMPONENT_TYPE_LIST*...)> theJob);

	// Add a component to the entity, return success or not.
	// etc. it the ID is invalid, it will return false,
	// but if the space of the componentManager is not enough the assertion will happen.
	template<typename COMPONENT_TYPE, typename ...COMPONENT_CONSTRUCT_ARGS>
	bool AttachTo(EntityID targetID, COMPONENT_CONSTRUCT_ARGS&&...args);

	// Detach a component from the entity, return success or not.
	template<typename ...COMPONENT_TYPE_LIST>
	bool DetachFrom(EntityID targetID);

	// Check one entity, dose it have all the components?
	template<typename ...COMPONENT_TYPE_LIST>
	bool DoHave(EntityID targetID);

private:
	template<typename COMPONENT_TYPE>
	static ECS::ComponentManager<COMPONENT_TYPE>* getComponentManager();

	template<typename COMPONENT_TYPE>
	static COMPONENT_TYPE* getComponent(ECS::EntityID id);

	static ECS::EntityManager<EntityManagerTraits>* s_pEntityManager;
};

#pragma region ECSWorld static members definations

// Initialize static member.
template<typename EntityManagerTraits, typename ...ComponentType>
ECS::EntityManager<EntityManagerTraits>* ECSWorld<EntityManagerTraits, ComponentType...>::s_pEntityManager = ECS::EntityManager<EntityManagerTraits>::getInstance();

#pragma endregion

#pragma region ECSWorld function definations

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename ...COMPONENT_TYPE_LIST>
inline void ECSWorld<EntityManagerTraits, ComponentType...>::Foreach(std::function<void(EntityID id, COMPONENT_TYPE_LIST*...args)> theJob)
{
	static_assert(TypeTool::IsAllOf<TypeTool::TypeContainer<COMPONENT_TYPE_LIST...>, ComponentType...>::value,
		"Exist not supported component type");

	for (EntityID  id : s_pEntityManager->RangeEntities<COMPONENT_TYPE_LIST...>())
	{
		theJob(id, getComponent<COMPONENT_TYPE_LIST>(id)...);
	}
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename ...COMPONENT_TYPE_LIST>
inline bool ECSWorld<EntityManagerTraits, ComponentType...>::ForOne(EntityID id, std::function<void(COMPONENT_TYPE_LIST*...)> theJob)
{
	if ( ! s_pEntityManager->isValid(id))
	{
		// The id is wrong.
		return false;
	}

	theJob(getComponent<COMPONENT_TYPE_LIST>(id)...);

	return true;
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename COMPONENT_TYPE, typename ...COMPONENT_CONSTRUCT_ARGS>
inline bool ECSWorld<EntityManagerTraits, ComponentType...>::
AttachTo(EntityID targetID, COMPONENT_CONSTRUCT_ARGS && ...args)
{
	if ( ! s_pEntityManager->isValid(targetID))
		return false;

	COMPONENT_TYPE * pNewComponent = 
		ECSWorldAlias::getComponentManager<COMPONENT_TYPE>()
		->newComponnet<COMPONENT_CONSTRUCT_ARGS...>(targetID, std::forward<COMPONENT_CONSTRUCT_ARGS>(args)...);
	s_pEntityManager->maskComponentType<ComponentType...>(targetID);
	return true;
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename COMPONENT_TYPE>
inline ECS::ComponentManager<COMPONENT_TYPE>* ECSWorld<EntityManagerTraits, ComponentType...>::getComponentManager()
{
	static ComponentManager<COMPONENT_TYPE> s_componentManager(ECS::DefaultComponentManagerTraits::MaxSize);
	return &s_componentManager;
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename COMPONENT_TYPE>
inline COMPONENT_TYPE * ECSWorld<EntityManagerTraits, ComponentType...>::getComponent(ECS::EntityID id)
{
	static ComponentManager<COMPONENT_TYPE>* s_pAnotherManager = getComponentManager<COMPONENT_TYPE>();
	return s_pAnotherManager->getComponent(id);
}

template<typename EntityManagerTraits, typename ...ComponentType>
inline EntityID ECSWorld<EntityManagerTraits, ComponentType...>::NewEntity()
{
	return s_pEntityManager->newEntity();
}

#pragma endregion

}// namespace ECS
