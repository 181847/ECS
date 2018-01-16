#pragma once
#include "ECS.h"
#include "ComponentManager.h"


namespace ECS
{

// The ECSWorld is used to store all the ComponentManager,
// and provide a function to get them,
// all the ComponentManager is using the template feature, 
class ECSWorld
{
public:
	// Create a static ComponentManager and construct it with passed arguments.
	// Only the first call will construct the manager, the created manager will be stored
	// in an other template function.
	// You should create only one instance of the ComponentManager refer to the 
	// same type of component, for example:
	// _____code start
	// auto pTwoIntegerManager	= ECSWorld::newComponentManager<IntComponent>(1000, 100);
	// auto pTwoFloatManager	= ECSWorld::newComponentManager<IntComponent>(100, 2.33333);
	// _____code end
	// Above is two call with same ComponentType, but with different construct arguments.
	// the second call will cause the assertion.
	template<typename COMPONENT_TYPE, typename ...COMPONENT_MANAGER_CONSTRUCT_ARGS>
	static ComponentManager<COMPONENT_TYPE>* newComponentManager(COMPONENT_MANAGER_CONSTRUCT_ARGS...args);

	// Get the ComponentManager, 
	// You must call newComponentManager() before this function,
	// or you will only get a nullptr.
	template<typename COMPONENT_TYPE>
	static ComponentManager<COMPONENT_TYPE>* getComponentManager();

private:
	// This is the place to get or store the ComponentManager's pointer,
	// the reference 'isFirstCall' will return whether this is the first call.
	// Only the newComponentManager() can store pointer and only the first call.
	template<typename COMPONENT_TYPE>
	static ComponentManager<COMPONENT_TYPE>* getOrStoreManager(
		bool& isFirstCall, 
		ComponentManager<COMPONENT_TYPE>* pStoredManager = nullptr);
};


template<typename COMPONENT_TYPE, typename ...COMPONENT_MANAGER_CONSTRUCT_ARGS>
inline ComponentManager<COMPONENT_TYPE>* ECSWorld::newComponentManager(
	COMPONENT_MANAGER_CONSTRUCT_ARGS ...args)
{
	static ComponentManager<COMPONENT_TYPE> s_componnetManager(args...);

	// access the storage template function,
	// check that there is no previous call with the same componentType.
	bool isFirstCall = false;
	ComponentManager<COMPONENT_TYPE> * pComponentManager = &s_componnetManager;
	pComponentManager = getOrStoreManager(isFirstCall, pComponentManager);
	ASSERT(isFirstCall && "You can only call only once with the same ComponentType.");

	return pComponentManager;
}

template<typename COMPONENT_TYPE>
inline ComponentManager<COMPONENT_TYPE>* ECSWorld::getComponentManager()
{
	bool isFirstCall = false;
	ComponentManager<COMPONENT_TYPE> * pComponentManager = 
		pComponentManager = getOrStoreManager<COMPONENT_TYPE>(isFirstCall);
	// the getOrStoreManager() shouldn't be the first call,
	// if isFirstCall == true, it means that the componentManager haven't be construct.
	ASSERT(false == isFirstCall && "Error!! ComponentManager hasn't been constructed, please call newComponentManager() before get if.");
	return pComponentManager;
}

template<typename COMPONENT_TYPE>
inline ComponentManager<COMPONENT_TYPE>* ECSWorld::getOrStoreManager(
	bool & isFirstCall, 
	ComponentManager<COMPONENT_TYPE>* pStoredManager)
{
	// Init the storage pointer only the first time.
	static ComponentManager<COMPONENT_TYPE>* s_pComponentManager = pStoredManager;
	static bool ImFirstCall = true;

	isFirstCall = ImFirstCall;
	ImFirstCall = false;

	return s_pComponentManager;
}

}// namespace ECS
