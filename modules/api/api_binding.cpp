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
		this->Set("DEBUG",Value::NewInt(KR_LOG_DEBUG));
		this->Set("INFO",Value::NewInt(KR_LOG_INFO));
		this->Set("ERROR",Value::NewInt(KR_LOG_ERROR));
		this->Set("WARN",Value::NewInt(KR_LOG_WARN));

		// these are convenience methods so you can do:
		//
		// Titanium.API.debug("hello")
		//
		// or
		//
		// Titanium.API.log(Titanium.API.DEBUG,"hello")
		//
		this->SetMethod("debug", &APIBinding::_LogDebug);
		this->SetMethod("info", &APIBinding::_LogInfo);
		this->SetMethod("error", &APIBinding::_LogError);
		this->SetMethod("warn", &APIBinding::_LogWarn);
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
	void APIBinding::_LogInfo(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		std::string msg = arg1->ToString();
		int severity = KR_LOG_INFO;
		this->Log(severity,msg);
	}
	void APIBinding::_LogDebug(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		SharedString msg = arg1->DisplayString();
		int severity = KR_LOG_DEBUG;
		this->Log(severity, *msg);
	}
	void APIBinding::_LogWarn(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		SharedString msg = arg1->DisplayString();
		int severity = KR_LOG_WARN;
		this->Log(severity, *msg);
	}
	void APIBinding::_LogError(const ValueList& args, SharedValue result)
	{
		SharedValue arg1 = args.at(0);
		SharedString msg = arg1->DisplayString();
		int severity = KR_LOG_ERROR;
		this->Log(severity, *msg);
	}
	void APIBinding::_Log(const ValueList& args, SharedValue result)
	{
		if (args.size()==1)
		{
			SharedValue arg1 = args.at(0);
			SharedString msg = arg1->DisplayString();
			int severity = KR_LOG_INFO;
			this->Log(severity, *msg);
		}
		else if (args.size()==2)
		{
			int severity = KR_LOG_INFO;

			SharedValue arg1 = args.at(0);
			if (arg1->IsString())
			{
				std::string type = arg1->ToString();
				if (type == "DEBUG")
				{
					severity = KR_LOG_DEBUG;
				}
				else if (type == "INFO")
				{
					severity = KR_LOG_INFO;
				}
				else if (type == "WARN")
				{
					severity = KR_LOG_WARN;
				}
				else if (type == "ERROR")
				{
					severity = KR_LOG_ERROR;
				}
			}
			else if (arg1->IsInt())
			{
				severity = arg1->ToInt();
			}
			SharedValue arg2 = args.at(1);
			SharedString message = arg2->DisplayString();
			this->Log(severity, *message);
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

	void APIBinding::Log(int& severity, std::string& message)
	{
		//FIXME: this is temporary implementation
		const char *type;

		switch (severity)
		{
			case KR_LOG_DEBUG:
				type = "DEBUG";
				break;
			case KR_LOG_INFO:
				type = "INFO";
				break;
			case KR_LOG_ERROR:
				type = "ERROR";
				break;
			case KR_LOG_WARN:
				type = "WARN";
				break;
			default:
				type = "CUSTOM";
				break;
		}

#if(DEBUG && OS_OSX)
		NSLog(@"[%s] %s",type,message.c_str());
#else
		std::cout << "[" << type << "] " << message << std::endl;
#endif
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
		if (this->registrations.size() == 0) return;

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
