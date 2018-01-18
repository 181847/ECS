#pragma once

#include <bitset>
#include <MyTools\MyAssert.h>
#include <MyTools\TypeTool.h>

#pragma comment(lib, "ECS.lib")

namespace ECS
{

static const size_t gMaxComponentTypeCount	= 32;

	// seral key type in the ECS.
using EntityID			= size_t;
using ComponentTypeID	= size_t;
using ComponentMask		= std::bitset<gMaxComponentTypeCount>;

template<typename ...COMPONENT_TYPES>
ComponentMask getComponentMask();



// this struct have no memeber, 
// its main goal is to generat the unique number
// for each COMPONENT_TYPE.
struct ComponetTypeIDSeed {};
using ComponentIDGenerator =  TypeTool::IDGenerator<ComponetTypeIDSeed>;

	// starup the ECS
int init();

	// initialize the gECSLogger.
int initLogger();

template<typename ...COMPONENT_TYPES>
inline ComponentMask getComponentMask()
{
	static ComponentMask mask(0);
	static const bool args[] = { 
		(false), // in case that the COMPONENT_TYPES is zero.
		(mask |= 1ull << ComponentIDGenerator::IDOf<COMPONENT_TYPES>(), false)... };
	return mask;
}

}// namespace ECS