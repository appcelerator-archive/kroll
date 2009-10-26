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
#include <Poco/ScopedLock.h>

namespace kroll
{
	Poco::FileOutputStream* ProfiledBoundObject::stream = 0;
	Poco::Mutex ProfiledBoundObject::logMutex;
	void ProfiledBoundObject::SetStream(Poco::FileOutputStream* stream)
	{
		ProfiledBoundObject::stream = stream;
	}

	ProfiledBoundObject::ProfiledBoundObject(KObjectRef delegate) :
		KObject(delegate->GetType()),
		delegate(delegate),
		count(1)
	{
	}

	ProfiledBoundObject::~ProfiledBoundObject()
	{
	}

	bool ProfiledBoundObject::AlreadyWrapped(KValueRef value)
	{
		if (value->IsMethod()) {
			KMethodRef source = value->ToMethod();
			AutoPtr<ProfiledBoundMethod> po = source.cast<ProfiledBoundMethod>();
			return !po.isNull();

		} else if (value->IsList()) {
			KListRef source = value->ToList();
			AutoPtr<ProfiledBoundList> po = source.cast<ProfiledBoundList>();
			return !po.isNull();

		} else if (value->IsObject()) {
			KObjectRef source = value->ToObject();
			AutoPtr<ProfiledBoundObject> po = source.cast<ProfiledBoundObject>();
			return !po.isNull();

		} else {
			return true;
		}
	}

	KValueRef ProfiledBoundObject::Wrap(KValueRef value, std::string type)
	{
		if (AlreadyWrapped(value))
		{
			return value;
		}
		else if (value->IsMethod())
		{
			KMethodRef toWrap = value->ToMethod();
			KMethodRef wrapped = new ProfiledBoundMethod(toWrap, type);
			return Value::NewMethod(wrapped);
		}
		else if (value->IsList())
		{
			KListRef wrapped = new ProfiledBoundList(value->ToList());
			return Value::NewList(wrapped);
		}
		else if (value->IsObject())
		{
			KObjectRef wrapped = new ProfiledBoundObject(value->ToObject());
			return Value::NewObject(wrapped);
		}
		else
		{
			return value;
		}
	}

	void ProfiledBoundObject::Set(const char *name, KValueRef value)
	{
		std::string type = this->GetSubType(name);
		KValueRef result = ProfiledBoundObject::Wrap(value, type);

		Poco::Stopwatch sw;
		sw.start();
		delegate->Set(name, result);
		sw.stop();

		this->Log("set", type, sw.elapsed());
	}

	KValueRef ProfiledBoundObject::Get(const char *name)
	{
		std::string type = this->GetSubType(name);

		Poco::Stopwatch sw;
		sw.start();
		KValueRef value = delegate->Get(name);
		sw.stop();

		this->Log("get", type, sw.elapsed());
		return ProfiledBoundObject::Wrap(value, type);
	}

	SharedStringList ProfiledBoundObject::GetPropertyNames()
	{
		return delegate->GetPropertyNames();
	}

	void ProfiledBoundObject::Log(
		const char* eventType, std::string& name, Poco::Timestamp::TimeDiff elapsedTime)
	{
		Poco::ScopedLock<Poco::Mutex> lock(logMutex);
		if ((*ProfiledBoundObject::stream)) {
			*ProfiledBoundObject::stream << Host::GetElapsedTime() << ",";
			*ProfiledBoundObject::stream << eventType << ",";
			*ProfiledBoundObject::stream << name << ",";
			*ProfiledBoundObject::stream << elapsedTime << "," << std::endl;
		}
	}

	SharedString ProfiledBoundObject::DisplayString(int levels)
	{
		return delegate->DisplayString(levels);
	}

	bool ProfiledBoundObject::HasProperty(const char* name)
	{
		return delegate->HasProperty(name);
	}

	bool ProfiledBoundObject::Equals(KObjectRef other)
	{
		AutoPtr<ProfiledBoundObject> pother = other.cast<ProfiledBoundObject>();
		if (!pother.isNull())
			other = pother->GetDelegate();

		return other.get() == this->GetDelegate().get();
	}

	std::string ProfiledBoundObject::GetSubType(std::string name)
	{
		if (!this->GetType().empty())
		{
			return this->GetType() + "." + name;
		}
		else
		{
			return name;
		}
	}
}
