#pragma once
#include "ECS.h"


namespace ECS
{

// forward class declaration
// BaseComponentSecretDesigner is used to access to the 
// private/ protected member in the BAseComponent(such as the _hostID).
class BaseComponentSecretDesigner;

// this is th base Component type.
class BaseComponent
{
public:
	BaseComponent();
	~BaseComponent();

	EntityID getHostID();

private:
	EntityID _hostID;
	friend class BaseComponentSecretDesigner;
};

}// namespace ECS

