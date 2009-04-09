/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>
#include <Poco/Stopwatch.h>

namespace kroll
{
	ProfiledBoundObject::ProfiledBoundObject(std::string name, SharedKObject delegate, Poco::FileOutputStream *stream) : delegate(delegate), name(name), stream(stream)
	{
	}
	ProfiledBoundObject::~ProfiledBoundObject()
	{
	}
	std::string ProfiledBoundObject::MakeFullPath(ProfiledBoundObject* ref,std::string name)
	{
		std::string newname(ref->GetFullPath());
		if (!name.empty())
		{
			newname+=".";
			newname+=std::string(name);
		}
		return newname;
	}
	SharedValue ProfiledBoundObject::Wrap(ProfiledBoundObject* ref, const char *name, SharedValue value, ProfiledBoundObject **out)
	{
		if (value->IsMethod())
		{
			SharedKMethod source = value->ToMethod();
			SharedPtr<ProfiledBoundMethod> po = source.cast<ProfiledBoundMethod>();
			if (!po.isNull())
			{
				*out = po.get();
				return value;
			}
			std::string newname = MakeFullPath(ref,name);
			ProfiledBoundMethod *p = new ProfiledBoundMethod(newname,source,ref->stream);
			*out = p;
			SharedKMethod target = p;
			return Value::NewMethod(target);
		}
		else if (value->IsList())
		{
			SharedKList source = value->ToList();
			SharedPtr<ProfiledBoundList> po = source.cast<ProfiledBoundList>();
			if (!po.isNull())
			{
				*out = po.get();
				return value;
			}
			std::string newname = MakeFullPath(ref,name);
			ProfiledBoundList *p = new ProfiledBoundList(newname,source,ref->stream);
			*out = p;
			SharedKList target = p;
			return Value::NewList(target);
		}
		else if (value->IsObject())
		{
			SharedKObject source = value->ToObject();
			SharedPtr<ProfiledBoundObject> po = source.cast<ProfiledBoundObject>();
			if (!po.isNull())
			{
				*out = po.get();
				return value;
			}
			std::string newname = MakeFullPath(ref,name);
			ProfiledBoundObject *p = new ProfiledBoundObject(newname,source,ref->stream);
			*out = p;
			SharedKObject target = p;
			return Value::NewObject(target);
		}
		return value;
	}
	void ProfiledBoundObject::Set(const char *name, SharedValue value)
	{
		ProfiledBoundObject *po = NULL;
		SharedValue result = ProfiledBoundObject::Wrap(this,name,value,&po);
		Poco::Stopwatch sw;
		sw.start();
		delegate->Set(name,result);
		sw.stop();
		if (po)
		{
			this->Log("set,%s,%lld",po->GetFullPath().c_str(),sw.elapsed());
		}
	}
	SharedValue ProfiledBoundObject::Get(const char *name)
	{
		Poco::Stopwatch sw;
		sw.start();
		SharedValue value = delegate->Get(name);
		sw.stop();
		ProfiledBoundObject *po = NULL;
		SharedValue result = ProfiledBoundObject::Wrap(this,name,value,&po);
		if (po)
		{
			this->Log("get,%s,%lld",po->GetFullPath().c_str(),sw.elapsed());
		}
		return result;
	}
	SharedValue ProfiledBoundObject::ProfiledCall(ProfiledBoundObject* ref, SharedKMethod method, const ValueList& args)
	{
		Poco::Stopwatch sw;
		sw.start();
		SharedValue value;
		try
		{
			value = method->Call(args);
		}
		catch(...)
		{
			sw.stop();
			this->Log("call,%s,%lld",ref->GetFullPath().c_str(),sw.elapsed());
			throw;
		}
		sw.stop();
		this->Log("call,%s,%lld",ref->GetFullPath().c_str(),sw.elapsed());
		return value;
	}
	SharedStringList ProfiledBoundObject::GetPropertyNames()
	{
		return delegate->GetPropertyNames();
	}
	void ProfiledBoundObject::Log(const char *format, ...)
	{
		char buffer[256];
		va_list args;
		va_start (args,format);
		vsprintf (buffer,format,args);
		char ts[80];
		sprintf(ts,"%lld,",Host::GetElapsedTime());
		(*stream) << ts << buffer << "\n";
		va_end (args);
	}
}
