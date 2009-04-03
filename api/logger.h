#include <Poco/Logger.h>
#include <Poco/Message.h>

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
		static void Initialize(int, int, int, std::string);

		Logger() {};
		~Logger() {}
		Logger(std::string);
		Logger(bool, bool, int, std::string, std::string);

		Logger& GetChild(std::string name);
		std::string& GetName();
		void Log(Level, std::string);
		void Trace(std::string);
		void Debug(std::string);
		void Information(std::string);
		void Notice(std::string);
		void Warning(std::string);
		void Error(std::string);
		void Critical(std::string);
		void Fatal(std::string);

		private:
		std::string name;
		static Logger& GetImpl(std::string name);
		static std::map<std::string, Logger> loggers;
	};
}

