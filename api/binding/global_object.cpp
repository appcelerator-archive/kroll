/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
namespace kroll
{
	AutoPtr<GlobalObject> GlobalObject::instance;

	GlobalObject::GlobalObject() :
		KEventObject("")
	{
		// @tiapi(method=True,type=String,name=getVersion,since=0.8)
		// @tiapi Return the Titanium runtime version.
		// @tiresult[String] The runtime version.
		this->SetMethod("getVersion", &GlobalObject::GetVersion);

		// @tiapi(method=True,type=String,name=getPlatform,since=0.8)
		// @tiapi Return the current platform, usually one of
		// @tiapi 'osx', 'win32', or 'linux'.
		// @tiresult[String] The current platform.
		this->SetMethod("getPlatform", &GlobalObject::GetPlatform);

		Event::SetEventConstants(this);
		Script::Initialize();
	}

	GlobalObject::~GlobalObject()
	{

	}

	void GlobalObject::GetVersion(const ValueList& args, KValueRef result)
	{
		static std::string version(PRODUCT_VERSION);
		result->SetString(version);
	}

	void GlobalObject::GetPlatform(const ValueList& args, KValueRef result)
	{
		static std::string platform(Host::GetInstance()->GetPlatform());
		result->SetString(platform);
	}

	/*static*/
	void GlobalObject::TurnOnProfiling()
	{
		instance = new ProfiledGlobalObject(instance);
	}
}

