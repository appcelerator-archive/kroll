#include "kroll.h"

#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FileChannel.h>
#include <Poco/Path.h>

using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::SplitterChannel;
using Poco::ConsoleChannel;
using Poco::FileChannel;
using Poco::Path;

namespace kroll
{
	std::map<std::string, Logger> Logger::loggers;

	Logger& Logger::Get(std::string name)
	{
		name = std::string(PRODUCT_NAME) + "." + name;
		return Logger::GetImpl(name);
	}

	void Logger::Initialize(int console, int file, int level, std::string appID)
	{
		std::string name = PRODUCT_NAME;
		Logger::loggers[name] = Logger(console, file, level, name, appID);
	}

	Logger& Logger::GetRootLogger()
	{
		return Logger::GetImpl(PRODUCT_NAME);
	}

	Logger& Logger::GetImpl(std::string name)
	{
		if (loggers.find(name) == loggers.end())
		{
			loggers[name] = Logger(name);
		}
		return loggers[name];
	}

	/* The root logger */
	Logger::Logger(bool console, bool file, int level, std::string name, std::string appID) :
		name(name)
	{
		SplitterChannel* splitter = new SplitterChannel();
		if (console)
		{
			ConsoleChannel* consoleChannel = new ConsoleChannel();
			splitter->addChannel(consoleChannel);
			consoleChannel->release();
		}
		if (file)
		{
			Path logfile = Path(FileUtils::GetApplicationDataDirectory(appID));
			logfile = Path(logfile, "tiapp.log");
			FileChannel* fileChannel = new FileChannel(logfile.absolute().toString());
			splitter->addChannel(fileChannel);
			fileChannel->release();
		}

		PatternFormatter* formatter = new PatternFormatter("[%H:%M:%S:%i] [%s] [%p] %t");
		FormattingChannel* fChannel = new Poco::FormattingChannel(formatter);
		formatter->release();

		fChannel->setChannel(splitter);
		splitter->release();

		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.setChannel(fChannel);
		loggerImpl.setLevel(level);
		fChannel->release();
	}

	Logger::Logger(std::string name) :
		name(name)
	{
	}

	std::string& Logger::GetName()
	{
		return this->name;
	}

	Logger& Logger::GetChild(std::string name)
	{
		std::string childName = this->name + "." + name;
		return Logger::GetImpl(childName);
	}

	void Logger::Log(Level level, std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		Poco::Message m(this->name, message, (Poco::Message::Priority) level);
		loggerImpl.log(m);
	}

	void Logger::Trace(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.trace(message);
	}

	void Logger::Debug(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.debug(message);
	}

	void Logger::Information(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.information(message);
	}

	void Logger::Notice(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.notice(message);
	}

	void Logger::Warning(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.warning(message);
	}

	void Logger::Error(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.error(message);
	}

	void Logger::Critical(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.critical(message);
	}

	void Logger::Fatal(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.fatal(message);
	}
}
