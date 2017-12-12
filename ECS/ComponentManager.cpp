#include "ComponentManager.h"

namespace ECS
{

template<typename COMPONENT_TYPE>
ComponentManager<COMPONENT_TYPE>::ComponentManager(size_t maxComponentCount)
{
	
}

template<typename COMPONENT_TYPE>
ComponentManager<COMPONENT_TYPE>::~ComponentManager()
{
}

template<typename COMPONENT_TYPE>
Component * ComponentManager<COMPONENT_TYPE>::operator[](EntityID id)
{
	return nullptr;
}


}// namespace ECS
