/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <Poco/Logger.h>
#include <Poco/Message.h>
#include <Poco/Mutex.h>
#include <cstdarg>

namespace kroll
{
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

		static Logger& Get(std::string name);
		static Logger& GetRootLogger();
		static void Initialize(int, int, int, std::string, std::string logpath);

		Logger() {};
		~Logger() {};
		Logger(std::string);
		Logger(bool, bool, int, std::string, std::string);

		Logger& GetChild(std::string name);
		std::string& GetName();

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
		static Poco::Mutex mutex;
		static char buffer[];
		static std::string logpath;

		std::string name;
		static Logger& GetImpl(std::string name);
		static std::map<std::string, Logger> loggers;
	};
}

