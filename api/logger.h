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
		enum Level
		{
			LFATAL = Poco::Message::PRIO_FATAL,
			LCRITICAL = Poco::Message::PRIO_CRITICAL,
			LERROR = Poco::Message::PRIO_ERROR,
			LWARN = Poco::Message::PRIO_WARNING,
			LNOTICE = Poco::Message::PRIO_NOTICE,
			LINFO = Poco::Message::PRIO_INFORMATION,
			LDEBUG = Poco::Message::PRIO_DEBUG,
			LTRACE = Poco::Message::PRIO_TRACE
		};

		static Logger* Get(std::string name);
		static Logger* GetRootLogger();
		static void Initialize(bool, std::string, Level);
		static void Shutdown();

		Logger() {};
		virtual ~Logger() {};
		Logger(std::string);
		Logger(std::string, Level);

		Level GetLevel();
		std::string& GetName();
		Logger* GetChild(std::string name);
		Logger* GetParent();
		
		inline bool IsEnabled(Level);
		inline bool IsTraceEnabled();
		inline bool IsDebugEnabled();
		inline bool IsInfoEnabled();
		inline bool IsNoticeEnabled();
		inline bool IsWarningEnabled();
		inline bool IsErrorEnabled();
		inline bool IsCriticalEnabled();
		inline bool IsFatalEnabled();

		virtual void Log(Poco::Message m);
		void Log(Level, std::string &);
		void Log(Level, const char*, va_list);
		void Log(Level, const char*, ...);
		std::string Format(const char*, va_list);

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
		virtual void LogImpl(Poco::Message m);

		protected:
		bool consoleLogging;
		bool fileLogging;
		Poco::PatternFormatter* formatter;
		std::string logFilePath;
		std::ofstream logFile;

	};

}

