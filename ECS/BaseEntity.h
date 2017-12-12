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

private:
	EntityID _id;
	std::vector<BaseComponent*> _components;
};

}//namespace ECS

