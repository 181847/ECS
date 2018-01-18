#pragma once
#include "ECS.h"
#include "ComponentManager.h"


namespace ECS
{

// The ECSWorld is used to store all the ComponentManager,
// and provide a function to get them,
// all the ComponentManager is using the template feature.
template<typename EntityManagerTraits, typename ...ComponentManagerTraitsList>
class ECSWorld
{
public:

};

}// namespace ECS
