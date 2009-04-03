/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "api_binding.h"
#include <algorithm>

namespace kroll
{
	APIBinding::APIBinding(SharedKObject global) : record(0), global(global)
	{
		this->SetMethod("set", &APIBinding::_Set);
		this->SetMethod("get", &APIBinding::_Get);
		this->SetMethod("log", &APIBinding::_Log);
		this->SetMethod("register", &APIBinding::_Register);
		this->SetMethod("unregister", &APIBinding::_Unregister);
		this->SetMethod("fire", &APIBinding::_Fire);

		// these are properties for log severity levels
		this->Set("TRACE", Value::NewInt(Logger::LTRACE));
		this->Set("DEBUG", Value::NewInt(Logger::LDEBUG));
		this->Set("INFO", Value::NewInt(Logger::LINFO));
		this->Set("NOTICE", Value::NewInt(Logger::LNOTICE));
		this->Set("WARN", Value::NewInt(Logger::LWARN));
		this->Set("ERROR", Value::NewInt(Logger::LERROR));
		this->Set("CRITICAL", Value::NewInt(Logger::LCRITICAL));
		this->Set("FATAL", Value::NewInt(Logger::LFATAL));

		// these are convenience methods so you can do:
		// Titanium.API.debug("hello")
		// or
		// Titanium.API.log(Titanium.API.DEBUG,"hello")
		this->SetMethod("trace", &APIBinding::_LogTrace);
		this->SetMethod("debug", &APIBinding::_LogDebug);
		this->SetMethod("info", &APIBinding::_LogInfo);
		this->SetMethod("notice", &APIBinding::_LogNotice);
		this->SetMethod("warn", &APIBinding::_LogWarn);
		this->SetMethod("error", &APIBinding::_LogError);
		this->SetMethod("critical", &APIBinding::_LogCritical);
		this->SetMethod("fatal", &APIBinding::_LogFatal);
	}

	APIBinding::~APIBinding()
	{
		ScopedLock lock(&mutex);

		registrations.clear();
		registrationsById.clear();
	}

	int APIBinding::GetNextRecord()
	{
		ScopedLock lock(&mutex);
		return ++this->record;
	}

	void APIBinding::_Set(const ValueList& args, SharedValue result)
	{
		const char *key = args.at(0)->ToString();
		std::string s = key;
		SharedValue value = args.at(1);
		std::string::size_type pos = s.find_first_of(".");

		if (pos==std::string::npos)
		{
			this->Set(key, value);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			global->SetNS(key, value);
		}
	}

	void APIBinding::_Get(const ValueList& args, SharedValue result)
	{
		std::string s = args.at(0)->ToString();
		const char *key = s.c_str();
		SharedValue r = NULL;
		std::string::size_type pos = s.find_first_of(".");

		if (pos==std::string::npos)
		{
			r = this->Get(key);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			r = global->GetNS(key);
		}
		result->SetValue(r);
	}

	void APIBinding::_LogTrace(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LTRACE, msg);
	}
	void APIBinding::_LogDebug(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LDEBUG, msg);
	}
	void APIBinding::_LogInfo(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LINFO, msg);
	}
	void APIBinding::_LogNotice(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LNOTICE, msg);
	}
	void APIBinding::_LogWarn(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LWARN, msg);
	}
	void APIBinding::_LogError(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LERROR, msg);
	}
	void APIBinding::_LogCritical(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LCRITICAL, msg);
	}
	void APIBinding::_LogFatal(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		this->Log(Logger::LFATAL, msg);
	}

	void APIBinding::_Log(const ValueList& args, SharedValue result)
	{
		if (args.size() ==1)
		{
			SharedValue v = args.at(0);
			SharedString msg = v->DisplayString();
			this->Log(Logger::LINFO, *msg);
		}
		else if (args.size()==2)
		{
			int severity = Logger::LINFO;

			SharedValue arg1 = args.at(0);
			if (arg1->IsString())
			{
				std::string type = arg1->ToString();
				if (type == "TRACE")
					severity = Logger::LTRACE;

				else if (type == "DEBUG")
					severity = Logger::LDEBUG;

				else if (type == "INFO")
					severity = Logger::LINFO;

				else if (type == "NOTICE")
					severity = Logger::LNOTICE;

				else if (type == "WARN")
					severity = Logger::LWARN;

				else if (type == "ERROR")
					severity = Logger::LERROR;

				else if (type == "CRITICAL")
					severity = Logger::LCRITICAL;

				else if (type == "FATAL")
					severity = Logger::LFATAL;
			}
			else if (arg1->IsInt())
			{
				severity = arg1->ToInt();
			}
			SharedValue v = args.at(1);
			SharedString msg = v->DisplayString();
			this->Log(severity, *msg);
		}
	}

	void APIBinding::_Register(const ValueList& args, SharedValue result)
	{
		std::string event = args.at(0)->ToString();
		SharedKMethod method = args.at(1)->ToMethod();

		int id = this->Register(event, method);
		result->SetInt(id);
	}

	void APIBinding::_Unregister(const ValueList& args, SharedValue result)
	{
		int id = args.at(0)->ToInt();
		this->Unregister(id);
	}

	void APIBinding::_Fire(const ValueList& args, SharedValue result)
	{
		const char* event = args.at(0)->ToString();
		this->Fire(event,args.at(1));
	}

	//---------------- IMPLEMENTATION METHODS

	void APIBinding::Log(int severity, std::string& message)
	{
		Logger& l = Logger::Get("API");
		l.Log((Logger::Level) severity, message);
	}

	int APIBinding::Register(std::string& event, SharedKMethod callback)
	{
		ScopedLock lock(&mutex);
		int record = GetNextRecord();
		/* Fetch the records for this event. If the EventRecords
		 * doesn't exist in registrations, the STL map
		 * implementation will insert it into the map */
		std::string en(event);
		EventRecords records = this->registrations[en];
		records.push_back(callback);

		BoundEventEntry e;
		e.method = callback;
		e.event = en;
		this->registrationsById[record] = e;
		this->registrations[event] = records;

#ifdef DEBUG
		std::cout << "Register called for event: " << event << std::endl;
#endif
		return record;
	}

	void APIBinding::Unregister(int id)
	{
		ScopedLock lock(&mutex);
		std::map<int, BoundEventEntry>::iterator i = registrationsById.find(id);
		if (i == registrationsById.end())
		{
			return;
		}

		BoundEventEntry entry = i->second;
		EventRecords records = this->registrations[entry.event];
		EventRecords::iterator fi = records.begin();
		while (fi != records.end())
		{
			SharedKMethod callback = (*fi);
			if (callback.get() == entry.method.get())
			{
				records.erase(fi);
				break;
			}
			fi++;
		}
		if (records.size()==0)
		{
			this->registrations.erase(entry.event);
		}
		registrationsById.erase(id);
	}

	void APIBinding::Fire(const char* event, SharedValue value)
	{
#ifdef DEBUG		
		std::cout << "FIRING: " << event << std::endl;
#endif
		
		//TODO: might want to be a little more lenient on how we lock here
		ScopedLock lock(&mutex);
		
		// optimize even the lookup
		if (this->registrations.size() == 0)
			return;

		EventRecords records = this->registrations[event];
		if (records.size() > 0)
		{
			EventRecords::iterator i = records.begin();
			while (i != records.end())
			{
				SharedKMethod method = (*i++);
				method->Call(event,value);
			}
		}
	}

}
