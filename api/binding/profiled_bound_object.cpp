/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <Poco/Stopwatch.h>

namespace kroll
{
	ProfiledBoundObject::ProfiledBoundObject(std::string name, SharedKObject delegate, Poco::FileOutputStream *stream) :
		KObject(delegate->GetType()), delegate(delegate), name(name), stream(stream)
	{
	}

	ProfiledBoundObject::~ProfiledBoundObject()
	{
	}

	std::string ProfiledBoundObject::MakeFullPath(ProfiledBoundObject* ref, std::string name, bool useParent)
	{
		std::string newname(ref->GetFullPath());
		if (useParent)
		{
			newname = newname.substr(0,newname.rfind("."));
		}
		
		if (!name.empty())
		{
			newname+=".";
			newname+=std::string(name);
		}
		return newname;
	}

	SharedValue ProfiledBoundObject::Wrap(ProfiledBoundObject* ref, std::string name, SharedValue value, ProfiledBoundObject **out, bool useParent)
	{
		std::string newname = MakeFullPath(ref, name, useParent);

		if (value->IsMethod()) {

			SharedKMethod source = value->ToMethod();
			SharedPtr<ProfiledBoundMethod> po = source.cast<ProfiledBoundMethod>();
			if (!po.isNull())
			{
				*out = po.get();
				return value;
			}
			ProfiledBoundMethod *p = new ProfiledBoundMethod(newname,source,ref->stream);
			*out = p;
			SharedKMethod target = p;
			return Value::NewMethod(target);

		} else if (value->IsList()) {
			SharedKList source = value->ToList();
			SharedPtr<ProfiledBoundList> po = source.cast<ProfiledBoundList>();
			if (!po.isNull())
			{
				*out = po.get();
				return value;
			}
			ProfiledBoundList *p = new ProfiledBoundList(newname,source,ref->stream);
			*out = p;
			SharedKList target = p;
			return Value::NewList(target);

		} else if (value->IsObject()) {
			SharedKObject source = value->ToObject();
			SharedPtr<ProfiledBoundObject> po = source.cast<ProfiledBoundObject>();
			if (!po.isNull())
			{
				*out = po.get();
				return value;
			}
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
			std::ostringstream o;
			o << "set," << po->GetFullPath() << "," << sw.elapsed();
			this->Log(o.str());
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
			std::ostringstream o;
			o << "get," << po->GetFullPath() << "," << sw.elapsed();
			this->Log(o.str());
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
			std::ostringstream o;
			o << "call," << ref->GetFullPath() << "," << sw.elapsed();
			this->Log(o.str());
			throw;
		}
		sw.stop();
		std::ostringstream o;
		o << "call," << ref->GetFullPath() << "," << sw.elapsed();
		this->Log(o.str());
		
		if (!value.isNull() && value->IsObject())
		{
			ProfiledBoundObject *po = NULL;
			SharedValue newValue = ProfiledBoundObject::Wrap(ref, value->GetType(), value, &po, true);
			
			return newValue;
		}
		return value;
	}

	SharedStringList ProfiledBoundObject::GetPropertyNames()
	{
		return delegate->GetPropertyNames();
	}

	void ProfiledBoundObject::Log(std::string str)
	{
		(*stream) << Host::GetElapsedTime() << "," << str << "\n";
	}

	SharedString ProfiledBoundObject::DisplayString(int levels)
	{
		return delegate->DisplayString(levels);
	}

	bool ProfiledBoundObject::HasProperty(const char* name)
	{
		return delegate->HasProperty(name);
	}
}
