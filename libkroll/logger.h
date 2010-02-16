/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <Poco/Logger.h>
#include <Poco/Message.h>
#include <Poco/Mutex.h>
#include <Poco/PatternFormatter.h>
#include <cstdarg>
#include <iostream>
#include <fstream>

namespace kroll
{
	class RootLogger;
	class KROLL_API Logger
	{
		public:
		typedef enum
		{
			LFATAL = Poco::Message::PRIO_FATAL,
			LCRITICAL = Poco::Message::PRIO_CRITICAL,
			LERROR = Poco::Message::PRIO_ERROR,
			LWARN = Poco::Message::PRIO_WARNING,
			LNOTICE = Poco::Message::PRIO_NOTICE,
			LINFO = Poco::Message::PRIO_INFORMATION,
			LDEBUG = Poco::Message::PRIO_DEBUG,
			LTRACE = Poco::Message::PRIO_TRACE
		} Level;
		typedef void (*LoggerCallback)(Level, std::string&);

		static Logger* Get(std::string name);
		static Logger* GetRootLogger();
		static void Initialize(bool, std::string, Level);
		static void Shutdown();
		static Level GetLevel(std::string& level);
		static void AddLoggerCallback(LoggerCallback callback);

		Logger() {};
		virtual ~Logger() {};
		Logger(std::string);
		Logger(std::string, Level);

		Level GetLevel();
		void SetLevel(Logger::Level level);
		std::string& GetName();
		Logger* GetChild(std::string name);
		Logger* GetParent();
		
		bool IsEnabled(Level);
		bool IsTraceEnabled();
		bool IsDebugEnabled();
		bool IsInfoEnabled();
		bool IsNoticeEnabled();
		bool IsWarningEnabled();
		bool IsErrorEnabled();
		bool IsCriticalEnabled();
		bool IsFatalEnabled();

		virtual void Log(Poco::Message& m);
		void Log(Level, std::string &);
		void Log(Level, const char*, va_list);
		void Log(Level, const char*, ...);
		static std::string Format(const char*, va_list);

		void Trace(std::string);
		void Trace(const char*, ...);

		void Debug(std::string);
		void Debug(const char*, ...);

		void Info(std::string);
		void Info(const char*, ...);

		void Notice(std::string);
		void Notice(const char*, ...);

		void Warn(std::string);
		void Warn(const char*, ...);

		void Error(std::string);
		void Error(const char*, ...);

		void Critical(std::string);
		void Critical(const char*, ...);

		void Fatal(std::string);
		void Fatal(const char*, ...);

		protected:
		std::string name;
		Level level;
		static Poco::Mutex mutex;
		static char buffer[];

		static Logger* GetImpl(std::string name);
		static std::map<std::string, Logger*> loggers;
	};

	class KROLL_API RootLogger : public Logger
	{
		public:
		RootLogger(bool, std::string, Level);
		~RootLogger();
		static RootLogger* instance;
		virtual void LogImpl(Poco::Message& m);
		void AddLoggerCallback(LoggerCallback callback);

		protected:
		bool consoleLogging;
		bool fileLogging;
		Poco::PatternFormatter* formatter;
		std::string logFilePath;
		std::ofstream logFile;
		Poco::Mutex mutex;
		std::vector<LoggerCallback> callbacks;
	};

}

