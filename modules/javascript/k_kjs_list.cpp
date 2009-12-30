/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KKJSList::KKJSList(JSContextRef context, JSObjectRef jsobject) :
		KList("JavaScript.KKJSList"),
		context(NULL),
		jsobject(jsobject)
	{
		/* KJS methods run in the global context that they originated from
		 * this seems to prevent nasty crashes from trying to access invalid
		 * contexts later. Global contexts need to be registered by all modules
		 * that use a KJS context. */
		JSObjectRef globalObject = JSContextGetGlobalObject(context);
		JSGlobalContextRef globalContext = KJSUtil::GetGlobalContext(globalObject);

		// This context hasn't been registered. Something has gone pretty
		// terribly wrong and Kroll will likely crash soon. Nonetheless, keep
		// the user up-to-date to keep their hopes up.
		if (globalContext == NULL)
			std::cerr << "Could not locate global context for a KJS method."  <<
				" One of the modules is misbehaving." << std::endl;
		this->context = globalContext;

		KJSUtil::ProtectGlobalContext(this->context);
		JSValueProtect(this->context, this->jsobject);

		this->kobject = new KKJSObject(this->context, this->jsobject);
	}

	KKJSList::~KKJSList()
	{
		JSValueUnprotect(this->context, this->jsobject);
		KJSUtil::UnprotectGlobalContext(this->context);
	}

	unsigned int KKJSList::Size()
	{
		KValueRef length_val = this->kobject->Get("length");
		if (length_val->IsInt())
			return (unsigned int) length_val->ToInt();
		else
			return 0;
	}

	KValueRef KKJSList::At(unsigned int index)
	{
		std::string name = KList::IntToChars(index);
		KValueRef value = this->kobject->Get(name.c_str());
		return value;
	}

	void KKJSList::SetAt(unsigned int index, KValueRef value)
	{
		std::string name = KList::IntToChars(index);
		this->kobject->Set(name.c_str(), value);
	}

	void KKJSList::Append(KValueRef value)
	{
		KValueRef push_method = this->kobject->Get("push");

		if (push_method->IsMethod())
		{
			ValueList list;
			list.push_back(value);
			push_method->ToMethod()->Call(list);
		}
		else
		{
			throw ValueException::FromString("Could not find push method on KJS array.");
		}
	}

	bool KKJSList::Remove(unsigned int index)
	{
		if (index >= 0 && index < this->Size())
		{
			KValueRef spliceMethod = this->kobject->Get("splice");
			spliceMethod->ToMethod()->Call(
				Value::NewInt(index),
				Value::NewInt(1));
			return true;
		}
		return false;
	}


	KValueRef KKJSList::Get(const char* name)
	{
		return kobject->Get(name);
	}

	void KKJSList::Set(const char* name, KValueRef value)
	{
		return kobject->Set(name, value);
	}

	bool KKJSList::Equals(KObjectRef other)
	{
		return this->kobject->Equals(other);
	}

	SharedStringList KKJSList::GetPropertyNames()
	{
		 return kobject->GetPropertyNames();
	}

	bool KKJSList::HasProperty(const char* name)
	{
		return kobject->HasProperty(name);
	}

	bool KKJSList::SameContextGroup(JSContextRef c)
	{
		return kobject->SameContextGroup(c);
	}

	JSObjectRef KKJSList::GetJSObject()
	{
		return this->jsobject;
	}
}
