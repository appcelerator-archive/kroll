/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "component_binding.h"

namespace kroll
{

	ComponentBinding::ComponentBinding(SharedComponent component) :
		component(component)
	{
		this->SetMethod("getType", &ComponentBinding::_GetType);
		this->SetMethod("getName", &ComponentBinding::_GetName);
		this->SetMethod("getVersion", &ComponentBinding::_GetVersion);
		this->SetMethod("getPath", &ComponentBinding::_GetPath);
		this->SetMethod("getManifest", &ComponentBinding::_GetManifest);
		this->SetMethod("isBundled", &ComponentBinding::_IsBundled);
		this->SetMethod("isLoaded", &ComponentBinding::_IsLoaded);
	}

	void ComponentBinding::_GetType(const ValueList& args, SharedValue result)
	{
		result->SetInt((int) this->component->type);
	}

	void ComponentBinding::_GetName(const ValueList& args, SharedValue result)
	{
		result->SetString(this->component->name);
	}

	void ComponentBinding::_GetVersion(const ValueList& args, SharedValue result)
	{
		result->SetString(this->component->version);
	}

	void ComponentBinding::_GetPath(const ValueList& args, SharedValue result)
	{
		result->SetString(this->component->path);
	}

	void ComponentBinding::_GetManifest(const ValueList& args, SharedValue result)
	{

	}

	void ComponentBinding::_IsBundled(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->component->bundled);
	}

	void ComponentBinding::_IsLoaded(const ValueList& args, SharedValue result)
	{
		SharedApplication app = Host::GetInstance()->GetApplication();
		result->SetBool(false);

		if (this->component.get() == app->runtime.get())
		{
			result->SetBool(true);
		}

		std::vector<SharedComponent>::iterator i = app->modules.begin();
		while (i != app->modules.end())
		{
			SharedComponent c = *i++;
			if (c.get() == this->component.get())
			{
				result->SetBool(true);
			}
		}
	}
}
