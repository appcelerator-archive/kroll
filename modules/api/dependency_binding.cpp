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
		dependency(dependency)
	{
		this->SetMethod("getType", &DependencyBinding::_GetType);
		this->SetMethod("getName", &DependencyBinding::_GetName);
		this->SetMethod("getVersion", &DependencyBinding::_GetVersion);
		this->SetMethod("getRequirement", &DependencyBinding::_GetRequirement);
	}

	SharedDependency DependencyBinding::GetDependency()
	{
		return dependency;
	}

	void DependencyBinding::_GetType(const ValueList& args, SharedValue result)
	{
		result->SetInt((int) this->dependency->type);
	}

	void DependencyBinding::_GetName(const ValueList& args, SharedValue result)
	{
		result->SetString(this->dependency->name);
	}

	void DependencyBinding::_GetVersion(const ValueList& args, SharedValue result)
	{
		result->SetString(this->dependency->version);
	}

	void DependencyBinding::_GetRequirement(const ValueList& args, SharedValue result)
	{
		result->SetInt(this->dependency->requirement);
	}
}
