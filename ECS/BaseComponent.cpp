#include "BaseComponent.h"

namespace ECS
{


BaseComponent::BaseComponent()
{
}

BaseComponent::~BaseComponent()
{
}

EntityID ECS::BaseComponent::getHostID()
{
	return _hostID;
}


}// namespace ECS
