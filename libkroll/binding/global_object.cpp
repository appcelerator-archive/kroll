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
		KEventObject(PRODUCT_NAME)
	{
		this->SetMethod("getVersion", &GlobalObject::GetVersion);
		this->SetMethod("getPlatform", &GlobalObject::GetPlatform);

		Event::SetEventConstants(this);
	}

	GlobalObject::~GlobalObject()
	{

	}

	void GlobalObject::GetVersion(const ValueList& args, KValueRef result)
	{
		result->SetString(PRODUCT_VERSION);
	}

	void GlobalObject::GetPlatform(const ValueList& args, KValueRef result)
	{
		result->SetString(OS_NAME);
	}

	/*static*/
	void GlobalObject::TurnOnProfiling()
	{
		instance = new ProfiledGlobalObject(instance);
	}
}

