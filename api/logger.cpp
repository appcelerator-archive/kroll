/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "kroll.h"

#include <cstdarg>
#include <cstdio>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FileChannel.h>
#include <Poco/Path.h>
#include <Poco/File.h>

#define LOGGER_MAX_ENTRY_SIZE 512

using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::SplitterChannel;
using Poco::ConsoleChannel;
using Poco::FileChannel;
using Poco::Path;
using Poco::File;

namespace kroll
{
	std::map<std::string, Logger> Logger::loggers;
	char Logger::buffer[LOGGER_MAX_ENTRY_SIZE];
	Poco::Mutex Logger::mutex;
	std::string Logger::logpath;

	Logger& Logger::Get(std::string name)
	{
		name = std::string(PRODUCT_NAME) + "." + name;
		return Logger::GetImpl(name);
	}

	void Logger::Initialize(int console, int file, int level, std::string appID, std::string logpath)
	{
		std::string name = PRODUCT_NAME;
		Logger::logpath = logpath;
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
			Path logfile;
			if (Logger::logpath.empty())
			{
				std::string dataPath = FileUtils::GetApplicationDataDirectory(appID);
				File dataPathFile = File(dataPath);
				dataPathFile.createDirectories();

				logfile = Path(dataPath);
				logfile = Path(logfile, "tiapp.log");
			}
			else
			{
				logfile = Path(logpath);
			}
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

	std::string Logger::Format(const char* format, va_list args)
	{
		// Protect the buffer
		Poco::Mutex::ScopedLock lock(this->mutex);

		vsnprintf(Logger::buffer, LOGGER_MAX_ENTRY_SIZE - 1, format, args);
		Logger::buffer[LOGGER_MAX_ENTRY_SIZE - 1] = '\0';
		std::string text = buffer;
		return text;
	}

	void Logger::Log(Level level, const char* format, va_list args)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);

		// Don't do formatting when this logger filters the message.
		// This prevents unecessary string manipulation.
		if (level <= (Level) loggerImpl.getLevel())
		{
			std::string messageText = Logger::Format(format, args);
			this->Log(level, messageText);
		}
	}

	void Logger::Log(Level level, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(level, format, args);
		va_end(args);
	}

	void Logger::Trace(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.trace(message);
	}

	void Logger::Trace(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LTRACE, format, args);
		va_end(args);
	}

	void Logger::Debug(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.debug(message);
	}

	void Logger::Debug(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LDEBUG, format, args);
		va_end(args);
	}

	void Logger::Info(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.information(message);
	}

	void Logger::Info(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LINFO, format, args);
		va_end(args);
	}

	void Logger::Notice(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.notice(message);
	}

	void Logger::Notice(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LNOTICE, format, args);
		va_end(args);
	}

	void Logger::Warn(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.warning(message);
	}

	void Logger::Warn(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LWARN, format, args);
		va_end(args);
	}

	void Logger::Error(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.error(message);
	}

	void Logger::Error(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LERROR, format, args);
		va_end(args);
	}

	void Logger::Critical(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.critical(message);
	}

	void Logger::Critical(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LCRITICAL, format, args);
		va_end(args);
	}

	void Logger::Fatal(std::string message)
	{
		Poco::Logger& loggerImpl = Poco::Logger::get(name);
		loggerImpl.fatal(message);
	}

	void Logger::Fatal(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		this->Log(LFATAL, format, args);
		va_end(args);
	}
}
