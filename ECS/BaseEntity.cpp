#include "BaseEntity.h"
#include <MyTools\MyAssert.h>

namespace ECS
{


BaseEntity::BaseEntity()
{
}


BaseEntity::~BaseEntity()
{
}

EntityID ECS::BaseEntity::getID()
{
	return _id;
}

void BaseEntity::addComponent(BaseComponent * component)
{
	auto mask = component->getComponentMask();
	// the entity should only have one type component
	ASSERT(_cmpMask & mask == 0);
	_cmpMask |= component->getComponentMask();
	_components.push_back(component);
}

void BaseEntity::removeComponent(BaseComponent * removedComponent)
{
	auto removedPosition =
		std::find(_components.begin(), _components.end(),
		[&removedComponent](BaseComponent * pcmp) { return pcmp == removedComponent; });

	if (removedPosition != _components.end())
	{
		_components.erase(removedPosition);

		// remove the mask
		_cmpMask &= ~(removedComponent->getComponentMask());
	}

	ASSERT(false && "you are try to remove a none exist component from the entity");
	
}

}// namespace ECS
