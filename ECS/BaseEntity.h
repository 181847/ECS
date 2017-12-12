#pragma once
#include "ECS.h"
#include "BaseComponent.h"

namespace ECS
{ 

class BaseEntity
{
public:
	BaseEntity();
	~BaseEntity();

	EntityID getID();
	
	void addComponent(BaseComponent * component);
	void removeComponent(BaseComponent * removedComponent);

private:
	EntityID _id;

	// mask to dicate which type of component it have.
	ComponentMask _cmpMask;
	
	std::vector<BaseComponent*> _components;
};

}//namespace ECS

