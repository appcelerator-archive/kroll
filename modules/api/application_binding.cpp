/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "api_binding.h"
#include "application_binding.h"
#include "component_binding.h"
#include "dependency_binding.h"

#ifdef _MSC_VER
#include <process.h>
#define GETPID  _getpid
#elif defined(__GNUC__)
#include <sys/types.h>
#include <unistd.h>
#define GETPID  getpid
#endif

namespace kroll
{

	ApplicationBinding::ApplicationBinding(SharedApplication application, bool current) :
		application(application),
		current(current)
	{
		this->SetMethod("getID", &ApplicationBinding::_GetID);
		this->SetMethod("getGUID", &ApplicationBinding::_GetGUID);
		this->SetMethod("getName", &ApplicationBinding::_GetName);
		this->SetMethod("getVersion", &ApplicationBinding::_GetVersion);
		this->SetMethod("getPath", &ApplicationBinding::_GetPath);
		this->SetMethod("getExecutablePath", &ApplicationBinding::_GetExecutablePath);
		this->SetMethod("getResourcesPath", &ApplicationBinding::_GetResourcesPath);
		this->SetMethod("getDataPath", &ApplicationBinding::_GetDataPath);
		this->SetMethod("getManifest", &ApplicationBinding::_GetManifest);
		this->SetMethod("getProperties", &ApplicationBinding::_GetProperties);

		// Things which, for now, only apply to the current application
		this->SetMethod("isCurrent", &ApplicationBinding::_IsCurrent);
		this->SetMethod("getPID", &ApplicationBinding::_GetPID);
		this->SetMethod("getArguments", &ApplicationBinding::_GetArguments);
		this->SetMethod("hasArgument", &ApplicationBinding::_HasArgument);
		this->SetMethod("getArgumentValue", &ApplicationBinding::_GetArgumentValue);

		// Application dependencies
		this->SetMethod("getDependencies", &ApplicationBinding::_GetDependencies);
		this->SetMethod("resolveDependencies", &ApplicationBinding::_ResolveDependencies);

		// Resolved components
		this->SetMethod("getComponents", &ApplicationBinding::_GetComponents);
		this->SetMethod("getModules", &ApplicationBinding::_GetModules);
		this->SetMethod("getRuntime", &ApplicationBinding::_GetRuntime);

		// Components available during resolution
		this->SetMethod("getAvailableComponents", &ApplicationBinding::_GetAvailableComponents);
		this->SetMethod("getAvailableModules", &ApplicationBinding::_GetAvailableModules);
		this->SetMethod("getAvailableRuntimes", &ApplicationBinding::_GetAvailableRuntimes);
		this->SetMethod("getBundledComponents", &ApplicationBinding::_GetBundledComponents);
		this->SetMethod("getBundledModules", &ApplicationBinding::_GetBundledModules);
		this->SetMethod("getBundledRuntimes", &ApplicationBinding::_GetBundledRuntimes);
	}

	void ApplicationBinding::_GetID(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->id);
	}

	void ApplicationBinding::_GetGUID(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->guid);
	}

	void ApplicationBinding::_GetName(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->name);
	}

	void ApplicationBinding::_GetVersion(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->version);
	}

	void ApplicationBinding::_GetPath(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->path);
	}

	void ApplicationBinding::_GetExecutablePath(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->GetExecutablePath());
	}

	void ApplicationBinding::_GetResourcesPath(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->GetResourcesPath());
	}

	void ApplicationBinding::_GetDataPath(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->GetDataPath());
	}

	void ApplicationBinding::_GetManifestPath(const ValueList& args, SharedValue result)
	{
		result->SetString(this->application->manifestPath);
	}

	void ApplicationBinding::_GetManifest(const ValueList& args, SharedValue result)
	{
		vector<pair<string, string> > manifest =
			BootUtils::ReadManifestFile(this->application->manifestPath);

		SharedKList manifestList = APIBinding::ManifestToKList(manifest);
		result->SetList(manifestList);
	}

	void ApplicationBinding::_GetProperties(const ValueList& args, SharedValue result)
	{
	}

	void ApplicationBinding::_IsCurrent(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->current);
	}

	void ApplicationBinding::_GetPID(const ValueList& args, SharedValue result)
	{
		if (this->current)
		{
			result->SetInt(GETPID());
		}
	}

	void ApplicationBinding::_GetArguments(const ValueList& args, SharedValue result)
	{
		std::vector<std::string> arguments = this->application->GetArguments();
		SharedKList argumentList = StaticBoundList::FromStringVector(arguments);
		result->SetList(argumentList);
	}

	void ApplicationBinding::_HasArgument(const ValueList& args, SharedValue result)
	{
		args.VerifyException("hasArgument", "s");
		string arg = args.at(0)->ToString();
		result->SetBool(this->application->HasArgument(arg));
	}

	void ApplicationBinding::_GetArgumentValue(const ValueList& args, SharedValue result)
	{
		args.VerifyException("getArgumentValue", "s");
		string arg = args.at(0)->ToString();
		result->SetString(this->application->GetArgumentValue(arg));
	}

	void ApplicationBinding::_GetDependencies(const ValueList& args, SharedValue result)
	{
		result->SetList(APIBinding::DependencyVectorToKList(
			this->application->dependencies));
	}

	void ApplicationBinding::_ResolveDependencies(const ValueList& args, SharedValue result)
	{
		std::vector<SharedDependency> unresolved = this->application->ResolveDependencies();
		result->SetList(APIBinding::DependencyVectorToKList(unresolved));
	}

	void ApplicationBinding::_GetComponents(const ValueList& args, SharedValue result)
	{
		// Do not use a reference here, because we don't want to modify the
		// application's modules list.
		std::vector<SharedComponent> components = this->application->modules;

		if (!this->application->runtime.isNull())
		{
			components.push_back(this->application->runtime);
		}
		SharedKList componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetModules(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent>& components = this->application->modules;
		SharedKList componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetRuntime(const ValueList& args, SharedValue result)
	{
		if (!this->application->runtime.isNull())
		{
			result->SetObject(new ComponentBinding(this->application->runtime));
		}
	}

	void ApplicationBinding::_GetAvailableComponents(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components);
		SharedKList componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetAvailableModules(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components);
		SharedKList componentList = APIBinding::ComponentVectorToKList(components, MODULE);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetAvailableRuntimes(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components);
		SharedKList componentList = APIBinding::ComponentVectorToKList(components, RUNTIME);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetBundledComponents(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components, true);
		SharedKList componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetBundledModules(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components, true);
		SharedKList componentList = APIBinding::ComponentVectorToKList(components, MODULE);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetBundledRuntimes(const ValueList& args, SharedValue result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components, true);
		SharedKList componentList = APIBinding::ComponentVectorToKList(components, RUNTIME);
		result->SetList(componentList);
	}
}
