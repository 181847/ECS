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

	// Wrap the IsAllOf struct to check whether types is in the ComponentType...,
	// or whether the ECSWorld support the CHECK_TYPES.
	// Usage:
	//		Asume our ECSWorld is ECSWorld<EMTraits, IntComponent, FloatComponent>,
	//		CheckTypeSupproted<	IntComponent				>::value		=> true
	//		CheckTypeSupproted<	FloatComponent				>::value		=> true
	//		CheckTypeSupproted<	IntComponent, FloatComponent>::value		=> true
	//		CheckTypeSupproted<								>::value		=> false
	//		CheckTypeSupproted<	int							>::value		=> false
	template<typename ...CHECK_TYPES>
	using CheckTypeSupported = TypeTool::IsAllOf<TypeTool::TypeContainer<CHECK_TYPES...>, ComponentType...>;

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
	bool DoHas(EntityID targetID);

private:
	template<typename COMPONENT_TYPE>
	static ECS::ComponentManager<COMPONENT_TYPE>* getComponentManager();

	template<typename COMPONENT_TYPE>
	static COMPONENT_TYPE* getComponent(ECS::EntityID id);

	static ECS::EntityManager<EntityManagerTraits>* s_pEntityManager;

	// Prevent from doing some work inside the Foreach or ForOne,
	// such cannot Detach a component from the entity inside a Foreach operation.
	// Every time we do some operation of the entityManager or the componentManager,
	// we must ensure the s_isLocked is false, (if not, assert),
	// then before we start the operation, set the s_isLocked to be true.
	static bool s_isLocked;

	// This struct used to manager the s_isLocked,
	// when it is constructed, ensure the s_isLocked is false,(if not, assert),
	// then set s_isLocked to be true, 
	// when it is deconstructed, set the s_isLocked to te false.
	// Prevent from doing some work inside the Foreach or ForOne,
	// such cannot Detach a component from the entity inside a Foreach operation.
	struct StaicLockGuard
	{
		StaicLockGuard();
		~StaicLockGuard();
	};
};

#pragma region ECSWorld static members definations

// Initialize static member.
template<typename EntityManagerTraits, typename ...ComponentType>
ECS::EntityManager<EntityManagerTraits>* ECSWorld<EntityManagerTraits, ComponentType...>::s_pEntityManager = ECS::EntityManager<EntityManagerTraits>::getInstance();

template<typename EntityManagerTraits, typename ...ComponentType>
bool ECSWorld<EntityManagerTraits, ComponentType...>::s_isLocked = false;

#pragma endregion

#pragma region ECSWorld function definations

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename ...COMPONENT_TYPE_LIST>
inline void ECSWorld<EntityManagerTraits, ComponentType...>::Foreach(std::function<void(EntityID id, COMPONENT_TYPE_LIST*...args)> theJob)
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE_LIST...>::value,
		"Exist not supported component type");

	StaicLockGuard avoidConfliction;

	for (EntityID  id : s_pEntityManager->RangeEntities<COMPONENT_TYPE_LIST...>())
	{
		theJob(id, getComponent<COMPONENT_TYPE_LIST>(id)...);
	}
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename ...COMPONENT_TYPE_LIST>
inline bool ECSWorld<EntityManagerTraits, ComponentType...>::ForOne(EntityID id, std::function<void(COMPONENT_TYPE_LIST*...)> theJob)
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE_LIST...>::value,
		"Exist not supported component type");

	StaicLockGuard avoidConfliction;

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
inline bool ECSWorld<EntityManagerTraits, ComponentType...>::AttachTo(EntityID targetID, COMPONENT_CONSTRUCT_ARGS && ...args)
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE>::value,
		"Not supported component type");

	StaicLockGuard avoidConfliction;

	assert(s_pEntityManager->isValid(targetID) && 
		"The entity you are operated on is invalid.");

	// Dose the entity already have the component?
	if (s_pEntityManager->haveComponent<COMPONENT_TYPE>(targetID))
	{
		// Yes, it has. Just return false tell the caller the operation failed.
		return false;
	}

	COMPONENT_TYPE * pNewComponent = 
		ECSWorldAlias::getComponentManager<COMPONENT_TYPE>()
		->newComponnet<COMPONENT_CONSTRUCT_ARGS...>(targetID, std::forward<COMPONENT_CONSTRUCT_ARGS>(args)...);
	s_pEntityManager->maskComponentType<COMPONENT_TYPE>(targetID);

	return true;
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename ...COMPONENT_TYPE_LIST>
inline bool ECSWorld<EntityManagerTraits, ComponentType...>::DetachFrom(EntityID targetID)
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE_LIST...>::value,
		"Not supported component type");

	StaicLockGuard avoidConfliction;

	assert(s_pEntityManager->isValid(targetID) &&
		"The entity you are operated on is invalid.");

	// Dose the entity have all the component type?
	if ( ! s_pEntityManager->haveComponent<COMPONENT_TYPE_LIST...>(targetID))
	{
		// No, the entity may lack some component type, or maybe it dosen't have any one.
		return false;// operation failed.
	}

	// unpack operation for each component type.
	bool arrayInit[] =
	{
		(
			getComponentManager<COMPONENT_TYPE_LIST>()->removeComponent(targetID)
		)...
	};

	s_pEntityManager->removeMask<COMPONENT_TYPE_LIST...>(targetID);

	return true;
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename ...COMPONENT_TYPE_LIST>
inline bool ECSWorld<EntityManagerTraits, ComponentType...>::DoHas(EntityID targetID)
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE_LIST...>::value,
		"Exist not supported component type");

	return s_pEntityManager->haveComponent<COMPONENT_TYPE_LIST...>(targetID);
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename COMPONENT_TYPE>
inline ECS::ComponentManager<COMPONENT_TYPE>* ECSWorld<EntityManagerTraits, ComponentType...>::getComponentManager()
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE>::value,
		"Not supported component type");

	static ComponentManager<COMPONENT_TYPE> s_componentManager(ECS::DefaultComponentManagerTraits::MaxSize);
	return &s_componentManager;
}

template<typename EntityManagerTraits, typename ...ComponentType>
template<typename COMPONENT_TYPE>
inline COMPONENT_TYPE * ECSWorld<EntityManagerTraits, ComponentType...>::getComponent(ECS::EntityID id)
{
	static_assert(CheckTypeSupported<COMPONENT_TYPE>::value,
		"Not supported component type");

	static ComponentManager<COMPONENT_TYPE>* s_pAnotherManager = getComponentManager<COMPONENT_TYPE>();
	return s_pAnotherManager->getComponent(id);
}

template<typename EntityManagerTraits, typename ...ComponentType>
inline EntityID ECSWorld<EntityManagerTraits, ComponentType...>::NewEntity()
{
	StaicLockGuard avoidConfliction;

	return s_pEntityManager->newEntity();
}

#pragma endregion

template<typename EntityManagerTraits, typename ...ComponentType>
inline ECSWorld<EntityManagerTraits, ComponentType...>::StaicLockGuard::StaicLockGuard()
{
	assert(ECSWorldAlias::s_isLocked == false && "Confiction happend, other operation is accessing the ECSWorld.");
	// set lock.
	s_isLocked = true;
}

template<typename EntityManagerTraits, typename ...ComponentType>
inline ECSWorld<EntityManagerTraits, ComponentType...>::StaicLockGuard::~StaicLockGuard()
{
	// free the lock.
	ECSWorldAlias::s_isLocked = false;
}

}// namespace ECS
