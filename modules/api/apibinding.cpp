/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include "apibinding.h"
#include <algorithm>

namespace kroll
{
	APIBinding::APIBinding() : record(0)
	{
		Value *version = new Value(1.0);
		ScopedDereferencer r(version);
		this->Set("version",version);
		this->SetMethod("bind",&APIBinding::Bind);
		this->SetMethod("log",&APIBinding::Log);
		this->SetMethod("register",&APIBinding::Register);
		this->SetMethod("unregister",&APIBinding::Unregister);
		this->SetMethod("fire",&APIBinding::Fire);
	}
	APIBinding::~APIBinding()
	{
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
	void APIBinding::Bind(const ValueList& args, Value *result, BoundObject *context_local)
	{
		std::string name = args.at(0)->ToString();
		Value *value = args.at(1);
		this->Bind(name,value);
	}
	void APIBinding::Log(const ValueList& args, Value *result, BoundObject *context_local)
	{
		int severity = args.at(0)->ToInt();
		std::string message = args.at(1)->ToString();
		this->Log(severity,message);
	}
	void APIBinding::Register(const ValueList& args, Value *result, BoundObject *context_local)
	{
		std::string event = args.at(0)->ToString();
		BoundMethod* method = args.at(1)->ToMethod();
		ScopedDereferencer r(method);
		int id = this->Register(event,method);
		result = new Value(id);
	}
	void APIBinding::Unregister(const ValueList& args, Value *result, BoundObject *context_local)
	{
		//TODO
	}
	void APIBinding::Fire(const ValueList& args, Value *result, BoundObject *context_local)
	{
		std::string event = args.at(0)->ToString();
		this->Fire(event,args.at(1));
	}

	//---------------- IMPLEMENTATION METHODS

	void APIBinding::Bind(std::string& name, Value *value)
	{
		ScopedLock lock(&mutex);
		this->Set(name.c_str(),(Value*)value);
	}
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
	void APIBinding::Fire(std::string& event, Value *value)
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
				KR_ADDREF(value);
				method->Call(args,NULL);
			}
		}
	}
}
