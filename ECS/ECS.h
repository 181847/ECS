#pragma once

#include <bitset>
#include <log4cplus\logger.h>
#include <MyTools\MyAssert.h>
#include <MyTools\TypeTool.h>

#pragma comment(lib, "log4cplusD.lib")
#pragma comment(lib, "ECS.lib")

namespace ECS
{
	// global logger
extern log4cplus::Logger gECSLogger;
static const std::string gLoggerLayout		= "%d{%m/%d/%y %H:%M:%S} -%m[%l]%n";
	// the ECS support up to {gMaxComponentTypeCount} different componentType.
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
	static bool args[] = { 
		(false), // in case that the COMPONENT_TYPES is zero.
		(mask |= 1ull << ComponentIDGenerator::getID<COMPONENT_TYPES>(), false)... };
	return mask;
}

}// namespace ECS