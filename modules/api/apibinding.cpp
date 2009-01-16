/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "apibinding.h"
#include <algorithm>

namespace kroll
{
	APIBinding::APIBinding(SharedPtr<BoundObject> global) : record(0), global(global)
	{
		//KR_ADDREF(global);
		SharedPtr<Value> version = new Value(1.0);
		this->Set((const char*)"version",version);
		this->SetMethod("set",&APIBinding::_Set);
		this->SetMethod("get",&APIBinding::_Get);
		this->SetMethod("log",&APIBinding::_Log);
		this->SetMethod("register",&APIBinding::_Register);
		this->SetMethod("unregister",&APIBinding::_Unregister);
		this->SetMethod("fire",&APIBinding::_Fire);
	}
	APIBinding::~APIBinding()
	{
		//KR_DECREF(global);
		ScopedLock lock(&mutex);
		std::map<std::string,EventRecords*>::iterator i = registrations.begin();
		while(i!=registrations.end())
		{
			std::pair<std::string,EventRecords*> p = (*i++);
			EventRecords::iterator ri = p.second->begin();
			while (ri!=p.second->end())
			{
				BoundMethod* method = (*ri++);
				KR_DECREF(method);
			}
		}
		registrations.clear();
		std::map<int,BoundEventEntry>::iterator i2 = registrationsById.begin();
		while(i2!=registrationsById.end())
		{
			std::pair<int,BoundEventEntry> p = (*i2++);
			KR_DECREF(p.second.method);
		}
		registrationsById.clear();
	}
	int APIBinding::GetNextRecord()
	{
		ScopedLock lock(&mutex);
		return this->record++;
	}
	void APIBinding::_Set(const ValueList& args, SharedPtr<Value> result)
	{
		const char *key = args.at(0)->ToString();
		std::string s = key;
		SharedPtr<Value> value = args.at(1);
		std::string::size_type pos = s.find_first_of(".");
		if (pos==std::string::npos)
		{
			this->Set(key,value);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			global->SetNS(key,value);
		}
	}
	void APIBinding::_Get(const ValueList& args, SharedPtr<Value> result)
	{
		std::string s = args.at(0)->ToString();
		const char *key = s.c_str();
		SharedPtr<Value> r = NULL;
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
		ScopedDereferencer r_dec(r);
		result->Set(r);
	}
	void APIBinding::_Log(const ValueList& args, SharedPtr<Value> result)
	{
		int severity = args.at(0)->ToInt();
		std::string message = args.at(1)->ToString();
		this->Log(severity,message);
	}
	void APIBinding::_Register(const ValueList& args, SharedPtr<Value> result)
	{
		std::string event = args.at(0)->ToString();
		BoundMethod* method = args.at(1)->ToMethod();
		int id = this->Register(event,method);
		result->Set(id);
	}
	void APIBinding::_Unregister(const ValueList& args, SharedPtr<Value> result)
	{
		int id = args.at(0)->ToInt();
		this->Unregister(id);
	}
	void APIBinding::_Fire(const ValueList& args, SharedPtr<Value> result)
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
	int APIBinding::Register(std::string& event,BoundMethod* callback)
	{
		int record = GetNextRecord();
		ScopedLock lock(&mutex);
		EventRecords* records = this->registrations[event];
		if (records==NULL)
		{
			records = new EventRecords;
			this->registrations[event] = records;
		}
		records->push_back(callback);
		KR_ADDREF(callback);

		BoundEventEntry e;
		e.method = callback;
		e.event = event;
		KR_ADDREF(callback);
		this->registrationsById[record] = e;

		return record;
	}
	void APIBinding::Unregister(int id)
	{
		ScopedLock lock(&mutex);
		std::map<int,BoundEventEntry>::iterator i = registrationsById.find(id);
		if (i!=registrationsById.end())
		{
			std::pair<int,BoundEventEntry> e = (*i++);
			EventRecords* records = this->registrations[e.second.event];
			if (records)
			{
				EventRecords::iterator fi = records->begin();
				while(fi!=records->end())
				{
					BoundMethod* m = (*fi);
					if (m == e.second.method)
					{
						records->erase(fi,fi+1);
						break;
					}
					fi++;
				}
			}
			registrationsById.erase(id);
		}
	}
	void APIBinding::Fire(std::string& event, SharedPtr<Value> value)
	{
		//TODO: might want to be a little more lenient on how we lock here
		ScopedLock lock(&mutex);
		EventRecords* records = this->registrations[event];
		if (records)
		{
			EventRecords::iterator i = records->begin();
			while (i!=records->end())
			{
				BoundMethod *method = (*i++);
				ValueList args;
				args.push_back(new Value(event));
				args.push_back(value);
				//KR_ADDREF(value);
				method->Call(args);
			}
		}
	}
}
