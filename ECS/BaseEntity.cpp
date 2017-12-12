#include "BaseEntity.h"

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

}// namespace ECS
