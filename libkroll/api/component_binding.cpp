/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "api_binding.h"
#include "component_binding.h"

namespace kroll
{
	ComponentBinding::ComponentBinding(SharedComponent component) :
		KAccessorObject("API.Component"),
		component(component)
	{
		/**
		 * @tiapi(method=True,name=API.Component.getType,since=0.4)
		 * @tiapi Get the type of this component
		 * @tiresult[Number] The type of this component (eg API.MODULE)
		 */
		this->SetMethod("getType", &ComponentBinding::_GetType);

		/**
		 * @tiapi(method=True,name=API.Component.getName,since=0.4)
		 * @tiapi Get the name of this component
		 * @tiresult[String] The name of this component
		 */
		this->SetMethod("getName", &ComponentBinding::_GetName);

		/**
		 * @tiapi(method=True,name=API.Component.getVersion,since=0.4)
		 * @tiapi Get the version of this component
		 * @tiresult[String] The version of this component
		 */
		this->SetMethod("getVersion", &ComponentBinding::_GetVersion);

		/**
		 * @tiapi(method=True,name=API.Component.getPath,since=0.4)
		 * @tiapi Get the path to this component
		 * @tiresult[String] The path to this component
		 */
		this->SetMethod("getPath", &ComponentBinding::_GetPath);

		/**
		 * @tiapi(method=True,name=API.Component.isBundled,since=0.4)
		 * @tiapi Whether or not this component is bundled with an application
		 * @tiresult[Boolean] True if this is a bundled component, false otherwise
		 */
		this->SetMethod("isBundled", &ComponentBinding::_IsBundled);

		/**
		 * @tiapi(method=True,name=API.Component.isLoaded,since=0.4)
		 * @tiapi Whether or not this component is currently loaded
		 * @tiresult[Boolean] True if this is a loaded component, false otherwise
		 */
		this->SetMethod("isLoaded", &ComponentBinding::_IsLoaded);
	}

	void ComponentBinding::_GetType(const ValueList& args, KValueRef result)
	{
		result->SetInt((int) this->component->type);
	}

	void ComponentBinding::_GetName(const ValueList& args, KValueRef result)
	{
		result->SetString(this->component->name);
	}

	void ComponentBinding::_GetVersion(const ValueList& args, KValueRef result)
	{
		result->SetString(this->component->version);
	}

	void ComponentBinding::_GetPath(const ValueList& args, KValueRef result)
	{
		result->SetString(this->component->path);
	}

	void ComponentBinding::_IsBundled(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->component->bundled);
	}

	void ComponentBinding::_IsLoaded(const ValueList& args, KValueRef result)
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
