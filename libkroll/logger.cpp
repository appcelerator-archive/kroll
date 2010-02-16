/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "kroll.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Path.h>
#include <Poco/File.h>

#define LOGGER_MAX_ENTRY_SIZE 2048

using Poco::PatternFormatter;
using Poco::Path;
using Poco::File;

namespace kroll
{
	std::map<std::string, Logger*> Logger::loggers;
	char Logger::buffer[LOGGER_MAX_ENTRY_SIZE];
	Poco::Mutex Logger::mutex;

	/*static*/
	Logger* Logger::Get(std::string name)
	{
		name = std::string(PRODUCT_NAME) + "." + name;
		return Logger::GetImpl(name);
	}

	/*static*/
	void Logger::Initialize(bool console, std::string logFilePath, Level level)
	{
		Logger::loggers[PRODUCT_NAME] = 
			new RootLogger(console, logFilePath, level);
	}

	/*static*/
	void Logger::Shutdown()
	{
		std::map<std::string, Logger*>::iterator i = loggers.begin();
		while (i != loggers.end())
		{
			Logger* l = (i++)->second;
			delete l;
		}
		loggers.clear();
	}

	/*static*/
	Logger* Logger::GetRootLogger()
	{
		return RootLogger::instance;
	}

	/*static*/
	void Logger::AddLoggerCallback(LoggerCallback callback)
	{
		RootLogger* rootLogger = 
			reinterpret_cast<RootLogger*>(GetRootLogger());
		rootLogger->AddLoggerCallback(callback);
	}

	/*static*/
	Logger* Logger::GetImpl(std::string name)
	{
		if (loggers.find(name) == loggers.end())
		{
			loggers[name] = new Logger(name);
		}
		return loggers[name];
	}

	Logger::Level Logger::GetLevel(std::string& levelString)
	{
		if (levelString == "TRACE")
			return Logger::LTRACE;
		else if (levelString == "DEBUG") 
			return Logger::LDEBUG;
		else if (levelString == "INFO") 
			return Logger::LINFO;
		else if (levelString == "NOTICE") 
			return Logger::LNOTICE;
		else if (levelString == "WARN") 
			return Logger::LWARN;
		else if (levelString == "ERROR") 
			return Logger::LERROR;
		else if (levelString == "CRITICAL") 
			return Logger::LCRITICAL;
		else if (levelString == "FATAL")
			return Logger::LFATAL;
		else if (Host::GetInstance()->DebugModeEnabled())
			return Logger::LDEBUG;
		else
			return Logger::LINFO;
	}

	Logger::Logger(std::string name) :
		name(name)
	{
		Logger* parent = this->GetParent();
		this->level = parent->GetLevel();
	}

	Logger::Logger(std::string name, Level level) :
		name(name),
		level(level)
	{ }

	std::string& Logger::GetName()
	{
		return this->name;
	}

	Logger::Level Logger::GetLevel()
	{
		return this->level;
	}
	
	void Logger::SetLevel(Logger::Level level)
	{
		this->level = level;
	}

	bool Logger::IsEnabled(Level level)
	{
		return level <= this->level;
	}

	bool Logger::IsTraceEnabled()
	{
		return this->IsEnabled(LTRACE);
	}

	bool Logger::IsDebugEnabled()
	{
		return this->IsEnabled(LDEBUG);
	}

	bool Logger::IsInfoEnabled()
	{
		return this->IsEnabled(LINFO);
	}

	bool Logger::IsNoticeEnabled()
	{
		return this->IsEnabled(LNOTICE);
	}

	bool Logger::IsWarningEnabled()
	{
		return this->IsEnabled(LWARN);
	}

	bool Logger::IsErrorEnabled()
	{
		return this->IsEnabled(LERROR);
	}

	bool Logger::IsCriticalEnabled()
	{
		return this->IsEnabled(LCRITICAL);
	}

	bool Logger::IsFatalEnabled()
	{
		return this->IsEnabled(LFATAL);
	}

	Logger* Logger::GetChild(std::string name)
	{
		std::string childName = this->name + "." + name;
		return Logger::GetImpl(childName);
	}

	Logger* Logger::GetParent()
	{
		size_t lastPeriodPos = this->name.rfind(".");
		if (lastPeriodPos == std::string::npos)
		{
			// in some cases this causes an infinite loop
			if (RootLogger::instance == NULL)
			{
				return NULL;
			}
			
			return Logger::GetRootLogger();
		}
		else
		{
			std::string parentName = this->name.substr(0, lastPeriodPos);
			return Logger::GetImpl(parentName);
		}
	}

	void Logger::Log(Poco::Message& m)
	{
		// This check only happens at the entry logger and never in it's
		// parents. This is so a child logger can have a more permissive level.
		if ((Level) m.getPriority() <= this->level)
		{
			RootLogger* root = RootLogger::instance;
			root->LogImpl(m);
		}
	}

	void Logger::Log(Level level, std::string& message)
	{
		Poco::Message m(this->name, message, (Poco::Message::Priority) level);
		this->Log(m);
	}

	void Logger::Log(Level level, const char* format, va_list args)
	{
		// Don't do formatting when this logger filters the message.
		// This prevents unecessary string manipulation.
		if (level <= this->level)
		{
			std::string messageText = Logger::Format(format, args);
			this->Log(level, messageText);
		}
	}

	/*static*/
	std::string Logger::Format(const char* format, va_list args)
	{
		// Protect the buffer
		Poco::Mutex::ScopedLock lock(mutex);

		vsnprintf(Logger::buffer, LOGGER_MAX_ENTRY_SIZE - 1, format, args);
		Logger::buffer[LOGGER_MAX_ENTRY_SIZE - 1] = '\0';
		std::string text = buffer;
		return text;
	}

	void Logger::Log(Level level, const char* format, ...)
	{
		if (IsEnabled(level))
		{
			va_list args;
			va_start(args, format);
			this->Log(level, format, args);
			va_end(args);
		}
	}

	void Logger::Trace(std::string message)
	{
		if (IsTraceEnabled())
		{
			this->Log(LTRACE, message);
		}
	}

	void Logger::Trace(const char* format, ...)
	{
		if (IsTraceEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LTRACE, format, args);
			va_end(args);
		}
	}

	void Logger::Debug(std::string message)
	{
		if (IsDebugEnabled())
		{
			this->Log(LDEBUG, message);
		}
	}

	void Logger::Debug(const char* format, ...)
	{
		if (IsDebugEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LDEBUG, format, args);
			va_end(args);
		}
	}

	void Logger::Info(std::string message)
	{
		if (IsInfoEnabled())
		{
			this->Log(LINFO, message);
		}
	}

	void Logger::Info(const char* format, ...)
	{
		if (IsInfoEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LINFO, format, args);
			va_end(args);
		}
	}

	void Logger::Notice(std::string message)
	{
		if (IsNoticeEnabled())
		{
			this->Log(LNOTICE, message);
		}
	}

	void Logger::Notice(const char* format, ...)
	{
		if (IsNoticeEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LNOTICE, format, args);
			va_end(args);
		}
	}

	void Logger::Warn(std::string message)
	{
		if (IsWarningEnabled())
		{
			this->Log(LWARN, message);
		}
	}

	void Logger::Warn(const char* format, ...)
	{
		if (IsWarningEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LWARN, format, args);
			va_end(args);
		}
	}

	void Logger::Error(std::string message)
	{
		if (IsErrorEnabled())
		{
			this->Log(LERROR, message);
		}
	}

	void Logger::Error(const char* format, ...)
	{
		if (IsErrorEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LERROR, format, args);
			va_end(args);
		}
	}

	void Logger::Critical(std::string message)
	{
		if (IsCriticalEnabled())
		{
			this->Log(LCRITICAL, message);
		}
	}

	void Logger::Critical(const char* format, ...)
	{
		if (IsCriticalEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LCRITICAL, format, args);
			va_end(args);
		}
	}

	void Logger::Fatal(std::string message)
	{
		if (IsFatalEnabled())
		{
			this->Log(LFATAL, message);
		}
	}

	void Logger::Fatal(const char* format, ...)
	{
		if (IsFatalEnabled())
		{
			va_list args;
			va_start(args, format);
			this->Log(LFATAL, format, args);
			va_end(args);
		}
	}

	RootLogger* RootLogger::instance = NULL;
	RootLogger::RootLogger(bool consoleLogging, std::string logFilePath, Level level) :
		Logger(PRODUCT_NAME, level),
		consoleLogging(consoleLogging),
		fileLogging(!logFilePath.empty())
	{
		RootLogger::instance = this;
		this->formatter = new PatternFormatter("[%H:%M:%S:%i] [%s] [%p] %t");

		if (fileLogging)
		{
			// Before opening the logfile, ensure that a parent directory exists
			string logDirectory = FileUtils::Dirname(logFilePath);
			File logDirectoryFile = File(logDirectory);
			logDirectoryFile.createDirectories();

#ifdef OS_WIN32
			this->logFile.open(UTF8ToWide(logFilePath).c_str(),
				 std::ios::out | std::ios::trunc);
#else
			this->logFile.open(logFilePath.c_str(),
				 std::ios::out | std::ios::trunc);
#endif

			// Couldn't open the file, perhaps there is contention?
			if (!this->logFile.is_open())
			{
				this->fileLogging = false;
			}
		}
	}

	RootLogger::~RootLogger()
	{
		if (fileLogging)
		{
			this->logFile.close();
		}
	}

	void RootLogger::LogImpl(Poco::Message& m)
	{
		Poco::Mutex::ScopedLock lock(mutex);
		Level level = (Level) m.getPriority();
		std::string line;
		this->formatter->format(m, line);

		if (fileLogging)
		{
			this->logFile << line << std::endl;
			this->logFile.flush();
		}

		if (consoleLogging)
		{
			printf("%s\n", line.c_str());
			fflush(stdout);
		}

		for (size_t i = 0; i < callbacks.size(); i++)
		{
			callbacks[i](level, line);
		}
	}

	void RootLogger::AddLoggerCallback(LoggerCallback callback)
	{
		Poco::Mutex::ScopedLock lock(mutex);
		callbacks.push_back(callback);
	}
}
