#pragma once

#include <bitset>
#include <MyTools\MyAssert.h>
#include <MyTools\TypeTool.h>

#pragma comment(lib, "ECS.lib")

namespace ECS
{

static const size_t gMaxComponentTypeCount	= 32;

	// seral key type in the ECS.
using EntityID			= std::size_t;
using ComponentTypeID	= size_t;

// this struct have no memeber, 
// its main goal is to generat the unique number
// for each COMPONENT_TYPE.
struct ComponetTypeIDSeed {};
using ComponentIDGenerator =  TypeTool::IDGenerator<ComponetTypeIDSeed>;

	// starup the ECS
int init();

	// initialize the gECSLogger.
int initLogger();

}// namespace ECS