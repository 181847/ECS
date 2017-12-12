#pragma once


#include <log4cplus\logger.h>
namespace ECS
{

typedef size_t EntityID;
// the ComponentMask is used to dicate
// which type of
typedef uint32_t ComponentMask;

log4cplus::Logger gECSLogger;
const std::string gLoggerLayout = "%d{%m/%d/%y %H:%M:%S} -%m[%l]%n";

// starup the ECS
int init();

// initialize the gECSLogger.
int initLogger();


}// namespace ECS