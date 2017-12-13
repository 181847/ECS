#include "ECS.h"
#include <log4cplus\consoleappender.h>
#include <log4cplus\layout.h>
#include <log4cplus\loggingmacros.h>


namespace ECS
{

log4cplus::Logger gECSLogger;
int init()
{
	// prepare the logger.
	initLogger();
	return 0;
}


int initLogger()
{
	using namespace log4cplus;
	using namespace log4cplus::helpers;

	SharedObjectPtr<Appender> consoleAppender(new ConsoleAppender());
	consoleAppender->setName("ECSConsoleAppender");
	consoleAppender->setLayout(std::make_unique<PatternLayout>(
		LOG4CPLUS_TEXT(gLoggerLayout)));
	
	gECSLogger = log4cplus::Logger::getInstance("ECSLogger");
	gECSLogger.addAppender(consoleAppender);
	
	return 0;
}
}// namespace ECS