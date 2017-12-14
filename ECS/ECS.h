#pragma once

#include <bitset>
#include <log4cplus\logger.h>
#include <MyTools\MyAssert.h>

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

	// starup the ECS
int init();

	// initialize the gECSLogger.
int initLogger();

}// namespace ECS