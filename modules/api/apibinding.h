/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _API_BINDING_H_
#define _API_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>

namespace kroll
{
	typedef std::vector<BoundMethod*> EventRecords;

	struct BoundEventEntry
	{
		BoundMethod *method;
		std::string event;
	};

	enum LogSeverity 
	{
		KR_LOG_DEBUG = 1,
		KR_LOG_INFO = 2,
		KR_LOG_ERROR = 3,
		KR_LOG_WARN = 4
	};

	class APIBinding : public StaticBoundObject
	{
	public:
		APIBinding();
		virtual ~APIBinding();

		void Bind(const ValueList& args, Value *result, BoundObject *context_local);
		void Log(const ValueList& args, Value *result, BoundObject *context_local);
		void Register(const ValueList& args, Value *result, BoundObject *context_local);
		void Unregister(const ValueList& args, Value *result, BoundObject *context_local);
		void Fire(const ValueList& args, Value *result, BoundObject *context_local);

		void Bind(std::string& name, Value *value);
		void Log(int& severity, std::string& message);
		int Register(std::string& event, BoundMethod* callback);
		void Unregister(int ref);
		void Fire(std::string& event, Value *data);

		// void Unload(const ValueList& args, Value *result, BoundObject *context_local);
		// void Reload(const ValueList& args, Value *result, BoundObject *context_local);
		// void Modules(const ValueList& args, Value *result, BoundObject *context_local);
	private:
		std::map<std::string,EventRecords*> registrations;
		std::map<int,BoundEventEntry> registrationsById;
		Mutex mutex;
		int record;

		int GetNextRecord();
	};
}

#endif
