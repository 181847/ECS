#pragma once


#include <log4cplus\logger.h>
#include <MyTools\MyAssert.h>
#pragma comment(lib, "log4cplusD.lib")
#pragma comment(lib, "ECS.lib")
namespace ECS
{
typedef size_t EntityID;

extern log4cplus::Logger gECSLogger;
const std::string gLoggerLayout = "%d{%m/%d/%y %H:%M:%S} -%m[%l]%n";

// starup the ECS
int init();

// initialize the gECSLogger.
int initLogger();


}// namespace ECS