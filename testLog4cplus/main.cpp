#include <log4cplus\consoleappender.h>
#include <log4cplus\loggingmacros.h>
#include <log4cplus\configurator.h>
#include <log4cplus\logger.h>
#include <log4cplus\layout.h>

#pragma comment(lib, "log4cplusD.lib")

int main(int argc, char ** argv)
{

	using namespace log4cplus;
	using namespace log4cplus::helpers;
	SharedObjectPtr<Appender> _append(new ConsoleAppender());
	_append->setName(LOG4CPLUS_TEXT("test appender"));
	_append->setLayout(std::make_unique<PatternLayout>(LOG4CPLUS_TEXT("%d- %m [%L]")));

	Logger _logger = Logger::getInstance(LOG4CPLUS_TEXT("test logger"));
	_logger.addAppender(_append);
	
	LOG4CPLUS_ERROR(_logger, LOG4CPLUS_TEXT("test error message"));
	LOG4CPLUS_WARN(_logger, LOG4CPLUS_TEXT("test warning message"));
	

	std::getchar();
	return 0;
}