#pragma once
#include "ECS.h"

namespace ECS
{

class BaseComponent
{
public:
	BaseComponent();
	~BaseComponent();

private:
	EntityID _entityID;
};


struct ComponentMaskGenerator
{
	template<typename DERIVED_COMPONENT>
	ComponentMask getMask();
};

}// namespace ECS
