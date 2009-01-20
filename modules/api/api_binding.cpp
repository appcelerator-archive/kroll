/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "api_binding.h"
#include <algorithm>

namespace kroll
{
	APIBinding::APIBinding(SharedBoundObject global) : record(0), global(global)
	{
		SharedValue version = Value::NewDouble(1.0);
		this->Set((const char*) "version", version);
		this->SetMethod("set", &APIBinding::_Set);
		this->SetMethod("get", &APIBinding::_Get);
		this->SetMethod("log", &APIBinding::_Log);
		this->SetMethod("register", &APIBinding::_Register);
		this->SetMethod("unregister", &APIBinding::_Unregister);
		this->SetMethod("fire", &APIBinding::_Fire);
	}

	APIBinding::~APIBinding()
	{
		std::cout << "before ~APiBinding: " << (void*)this <<std::endl;
		ScopedLock lock(&mutex);

		/* clear old registrations -- memory mangement will be
		 * handled by the SharedPtr implementation */
		registrations.clear();
		registrationsById.clear();
	}

	int APIBinding::GetNextRecord()
	{
		ScopedLock lock(&mutex);
		return this->record++;
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

	void APIBinding::_Log(const ValueList& args, SharedValue result)
	{
		int severity = args.at(0)->ToInt();
		std::string message = args.at(1)->ToString();
		this->Log(severity, message);
	}

	void APIBinding::_Register(const ValueList& args, SharedValue result)
	{
		std::string event = args.at(0)->ToString();
		BoundMethod* method = args.at(1)->ToMethod();
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
		std::string event = args.at(0)->ToString();
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

		std::cout << "[" << type << "] " << message << std::endl;
	}

	int APIBinding::Register(std::string& event, SharedBoundMethod callback)
	{
		int record = GetNextRecord();
		ScopedLock lock(&mutex);

		/* Fetch the records for this event. If the EventRecords
		 * doesn't exist in registrations, the STL map
		 * implementation will insert it into the map */
		EventRecords records = this->registrations[event];
		records.push_back(callback);

		BoundEventEntry e;
		e.method = callback;
		e.event = event;
		this->registrationsById[record] = e;

		return record;
	}

	void APIBinding::Unregister(int id)
	{
		ScopedLock lock(&mutex);
		std::map<int, BoundEventEntry>::iterator i = registrationsById.find(id);
		if (i == registrationsById.end())
			return;

		BoundEventEntry entry = i->second;
		EventRecords records = this->registrations[entry.event];
		EventRecords::iterator fi = records.begin();
		while (fi != records.end())
		{
			SharedBoundMethod callback = *fi;
			if (callback.get() == entry.method.get())
			{
				records.erase(fi);
				break;
			}
			fi++;
		}
		registrationsById.erase(id);
	}

	void APIBinding::Fire(std::string& event, SharedValue value)
	{
		//TODO: might want to be a little more lenient on how we lock here
		ScopedLock lock(&mutex);
		EventRecords records = this->registrations[event];

		EventRecords::iterator i = records.begin();
		while (i != records.end())
		{
			SharedBoundMethod method = *i;
			ValueList args;
			args.push_back(Value::NewString(event));
			args.push_back(value);
			method->Call(args);
		}

	}
}
