/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "dependency_binding.h"

namespace kroll
{
	DependencyBinding::DependencyBinding(SharedDependency dependency) :
		KAccessorObject("API.Dependency"),
		dependency(dependency)
	{
		/**
		 * @tiapi(method=True,name=API.Dependency.getType,since=0.4)
		 * @tiapi Get the type of this dependency (eg API.MODULE)
		 * @tiresult[Number] The type of this dependency
		 */
		this->SetMethod("getType", &DependencyBinding::_GetType);

		/**
		 * @tiapi(method=True,name=API.Dependency.getName,since=0.4)
		 * @tiapi Get the name of this dependency
		 * @tiresult[String] The name of this dependency
		 */
		this->SetMethod("getName", &DependencyBinding::_GetName);

		/**
		 * @tiapi(method=True,name=API.Dependency.getVersion,since=0.4)
		 * @tiapi Get the version of this dependency
		 * @tiresult[String] The version of this dependency
		 */
		this->SetMethod("getVersion", &DependencyBinding::_GetVersion);

		/**
		 * @tiapi(method=True,name=API.Dependency.getRequirement,since=0.4)
		 * @tiapi Get the requirement for this dependency (eg API.LTE)
		 * @tiresult[String] The requirement for this dependency
		 */
		this->SetMethod("getRequirement", &DependencyBinding::_GetRequirement);
	}

	SharedDependency DependencyBinding::GetDependency()
	{
		return dependency;
	}

	void DependencyBinding::_GetType(const ValueList& args, KValueRef result)
	{
		result->SetInt((int) this->dependency->type);
	}

	void DependencyBinding::_GetName(const ValueList& args, KValueRef result)
	{
		result->SetString(this->dependency->name);
	}

	void DependencyBinding::_GetVersion(const ValueList& args, KValueRef result)
	{
		result->SetString(this->dependency->version);
	}

	void DependencyBinding::_GetRequirement(const ValueList& args, KValueRef result)
	{
		result->SetInt(this->dependency->requirement);
	}
}
