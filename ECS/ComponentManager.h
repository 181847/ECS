#pragma once
#include "ECS.h"

namespace ECS
{

template<typename COMPONENT_TYPE>
class ComponentManager
{
public:
	ComponentManager(size_t maxComponentCount);
	~ComponentManager();

	// generator a newComponent;
	COMPONENT_TYPE* newComponent();

	void deleteComponent(EntityID id);
};

}// namespace ECS

