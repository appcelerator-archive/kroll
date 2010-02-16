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
		KAccessorObject("API.Application"),
		application(application),
		current(current)
	{
		/**
		 * @tiapi(method=True,name=API.Application.getID,since=0.4)
		 * @tiapi Get this application's ID
		 * @tiresult[String] The ID of this application
		 */
		this->SetMethod("getID", &ApplicationBinding::_GetID);

		/**
		 * @tiapi(method=True,name=API.Application.getGUID,since=0.4)
		 * @tiapi Get this application's GUID
		 * @tiresult[String] The GUID of this application
		 */
		this->SetMethod("getGUID", &ApplicationBinding::_GetGUID);

		/**
		 * @tiapi(method=True,name=API.Application.getName,since=0.4)
		 * @tiapi Get this application's name
		 * @tiresult[String] The name of this application
		 */
		this->SetMethod("getName", &ApplicationBinding::_GetName);

		/**
		 * @tiapi(method=True,name=API.Application.getVersion,since=0.4)
		 * @tiapi Get this application's version
		 * @tiresult[String] The version of this application
		 */
		this->SetMethod("getVersion", &ApplicationBinding::_GetVersion);

		/**
		 * @tiapi(method=True,name=API.Application.getPath,since=0.4)
		 * @tiapi Get this application's path
		 * @tiresult[String] The path of this application
		 */
		this->SetMethod("getPath", &ApplicationBinding::_GetPath);

		/**
		 * @tiapi(method=True,name=API.Application.getExecutablePath,since=0.4)
		 * @tiapi Get this application's executable
		 * @tiresult[String] The executable path of this application
		 */
		this->SetMethod("getExecutablePath", &ApplicationBinding::_GetExecutablePath);

		/**
		 * @tiapi(method=True,name=API.Application.getResourcesPath,since=0.4)
		 * @tiapi Get the path to this application's resources directory
		 * @tiresult[String] The path to this application's resources directory
		 */
		this->SetMethod("getResourcesPath", &ApplicationBinding::_GetResourcesPath);

		/**
		 * @tiapi(method=True,name=API.Application.getDataPath,since=0.4)
		 * @tiapi Get the path to this application's user data directory
		 * @tiresult[String] The path to this application's user data directory
		 */
		this->SetMethod("getDataPath", &ApplicationBinding::_GetDataPath);

		/**
		 * @tiapi(method=True,name=API.Application.getManifestPath,since=0.4)
		 * @tiapi Get the path to this application's manifest
		 * @tiresult[String] The path to this application's manifest
		 */
		this->SetMethod("getManifestPath", &ApplicationBinding::_GetManifestPath);

		/**
		 * @tiapi(method=True,name=API.Application.getManifest,since=0.4)
		 * @tiapi Get the contents of this application's manifest
		 * @tiresult[Array<Array<String>>] The contents of the manifest as an array of key-value strings
		 */
		this->SetMethod("getManifest", &ApplicationBinding::_GetManifest);

		// TODO: Add an accessor for this applications properties
		// this->SetMethod("getProperties", &ApplicationBinding::_GetProperties);

		// Things which, for now, only apply to the current application

		/**
		 * @tiapi(method=True,name=API.Application.isCurrent,since=0.4)
		 * @tiapi Returns whether or not this is the currently running application
		 * @tiresult[Boolean] True if this is the currently running application and false otherwise
		 */
		this->SetMethod("isCurrent", &ApplicationBinding::_IsCurrent);

		/**
		 * @tiapi(method=True,name=API.Application.getPID,since=0.4)
		 * @tiapi Retrieve the process identifier of this application
		 * @tiresult[String] The process identifier of this application if it is the current application or empty string otherwise
		 */
		this->SetMethod("getPID", &ApplicationBinding::_GetPID);

		/**
		 * @tiapi(method=True,name=API.Application.getArguments,since=0.4)
		 * @tiapi Retrieve a list of command-line arguments passed to this application
		 * @tiresult[Array<String>] The list of command-line arguments to this application or an empty array if it is not the currently running application.
		 */
		this->SetMethod("getArguments", &ApplicationBinding::_GetArguments);

		/**
		 * @tiapi(method=True,name=API.Application.hasArgument,since=0.4)
		 * @tiapi Determine if the given argument is in the argument list
		 * @tiarg[String, argument] The argument to search for
		 * @tiresult[Boolean] True if the argument was found, false otherwise
		 */
		this->SetMethod("hasArgument", &ApplicationBinding::_HasArgument);

		/**
		 * @tiapi(method=True,name=API.Application.getArgumentValue,since=0.4)
		 * @tiapi Find the value for a given argument. This method tries to be
		 * @tiapi a little smart about finding the argument. If you pass "param"
		 * @tiapi or "--param" as argument it will find value for --param=<value>
		 * @tiarg[String, argument] The argument to search for
		 * @tiresult[String] The value of the given argument or empty string if not found.
		 */
		this->SetMethod("getArgumentValue", &ApplicationBinding::_GetArgumentValue);

		// Application dependencies

		/**
		 * @tiapi(method=True,name=API.Application.getDependencies,since=0.4)
		 * @tiapi Get a list of this application's dependencies.
		 * @tiresult[Array<API.Dependency>] A list of API.Dependency for this application
		 */
		this->SetMethod("getDependencies", &ApplicationBinding::_GetDependencies);

		/**
		 * @tiapi(method=True,name=API.Application.resolveDependencies,since=0.4)
		 * @tiapi Attempt to resolve all the dependencies for this application,
		 * @tiapi which should be accessible through API.Application.getComponents
		 * @tiapi afterward. Note that the currently running applicaiton should 
		 * @tiapi already have it's dependencies resolved.
		 * @tiresult[Array<API.Dependency>] A list of unresolved dependencies
		 */
		this->SetMethod("resolveDependencies", &ApplicationBinding::_ResolveDependencies);

		// Resolved components

		/**
		 * @tiapi(method=True,name=API.Application.getComponents,since=0.4)
		 * @tiapi Return a list of resolved components for this application
		 * @tiresult[Array<API.Component>] A list of API.Component representing all resolved components
		 */
		this->SetMethod("getComponents", &ApplicationBinding::_GetComponents);

		/**
		 * @tiapi(method=True,name=API.Application.getModules,since=0.4)
		 * @tiapi Return a list of resolved modules for this application
		 * @tiresult[Array<API.Component>] A list of API.Component representing all resolved modules
		 */
		this->SetMethod("getModules", &ApplicationBinding::_GetModules);

		/**
		 * @tiapi(method=True,name=API.Application.getRuntime,since=0.4)
		 * @tiapi Return the resolved runtime for this application
		 * @tiresult[Array<API.Component>] An API.Component representing the resolved runtime or null if there is no resolved runtime.
		 */
		this->SetMethod("getRuntime", &ApplicationBinding::_GetRuntime);

		// Components available during resolution

		/**
		 * @tiapi(method=True,name=API.Application.getAvailableComponents,since=0.4)
		 * @tiapi Return a list of all available (installed and bundled) components for this application
		 * @tiresult[Array<API.Component>] A list of API.Components available to this application
		 */
		this->SetMethod("getAvailableComponents", &ApplicationBinding::_GetAvailableComponents);

		/**
		 * @tiapi(method=True,name=API.Application.getAvailableModules,since=0.4)
		 * @tiapi Return a list of all available (installed and bundled) modules for this application
		 * @tiresult[Array<API.Component>] A list of API.Component of module type available to this application
		 */
		this->SetMethod("getAvailableModules", &ApplicationBinding::_GetAvailableModules);

		/**
		 * @tiapi(method=True,name=API.Application.getAvailableRuntimes,since=0.4)
		 * @tiapi Return a list of all available (installed and bundled) runtimes for this application
		 * @tiresult[Array<API.Component>] A list of API.Component of runtime type available to this application
		 */
		this->SetMethod("getAvailableRuntimes", &ApplicationBinding::_GetAvailableRuntimes);

		/**
		 * @tiapi(method=True,name=API.Application.getBundledComponents,since=0.4)
		 * @tiapi Return a list of all components bundled with this application
		 * @tiresult[Array<API.Component>] A list of all API.Components bundled with this application
		 */
		this->SetMethod("getBundledComponents", &ApplicationBinding::_GetBundledComponents);

		/**
		 * @tiapi(method=True,name=API.Application.getBundledModules,since=0.4)
		 * @tiapi Return a list of all modules bundled with this application
		 * @tiresult[Array<API.Component>] A list of all API.Components of type module bundled with this application
		 */
		this->SetMethod("getBundledModules", &ApplicationBinding::_GetBundledModules);

		/**
		 * @tiapi(method=True,name=API.Application.getBundledRuntimes,since=0.4)
		 * @tiapi Return a list of all runtimes bundled with this application
		 * @tiresult[Array<API.Component>] A list of all API.Components of type runtime bundled with this application
		 */
		this->SetMethod("getBundledRuntimes", &ApplicationBinding::_GetBundledRuntimes);
	}

	void ApplicationBinding::_GetID(const ValueList& args, KValueRef result)
	{
		result->SetString(this->application->id);
	}

	void ApplicationBinding::_GetGUID(const ValueList& args, KValueRef result)
	{
		result->SetString(this->application->guid);
	}

	void ApplicationBinding::_GetName(const ValueList& args, KValueRef result)
	{
		result->SetString(this->application->name);
	}

	void ApplicationBinding::_GetVersion(const ValueList& args, KValueRef result)
	{
		result->SetString(this->application->version);
	}

	void ApplicationBinding::_GetPath(const ValueList& args, KValueRef result)
	{
		result->SetString(this->application->path);
	}

	void ApplicationBinding::_GetExecutablePath(const ValueList& args, KValueRef result)
	{
		string executablePath = this->application->GetExecutablePath();
		result->SetString(executablePath);
	}

	void ApplicationBinding::_GetResourcesPath(const ValueList& args, KValueRef result)
	{
		string resourcesPath = this->application->GetResourcesPath();
		result->SetString(resourcesPath);
	}

	void ApplicationBinding::_GetDataPath(const ValueList& args, KValueRef result)
	{
		string dataPath = this->application->GetDataPath();
		result->SetString(dataPath);
	}

	void ApplicationBinding::_GetManifestPath(const ValueList& args, KValueRef result)
	{
		result->SetString(this->application->manifestPath);
	}

	void ApplicationBinding::_GetManifest(const ValueList& args, KValueRef result)
	{
		vector<pair<string, string> > manifest =
			BootUtils::ReadManifestFile(this->application->manifestPath);

		KListRef manifestList = APIBinding::ManifestToKList(manifest);
		result->SetList(manifestList);
	}

	void ApplicationBinding::_IsCurrent(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->current);
	}

	void ApplicationBinding::_GetPID(const ValueList& args, KValueRef result)
	{
		if (this->current)
		{
			result->SetInt(GETPID());
		}
	}

	void ApplicationBinding::_GetArguments(const ValueList& args, KValueRef result)
	{
		std::vector<std::string> arguments = this->application->GetArguments();
		KListRef argumentList = StaticBoundList::FromStringVector(arguments);
		result->SetList(argumentList);
	}

	void ApplicationBinding::_HasArgument(const ValueList& args, KValueRef result)
	{
		args.VerifyException("hasArgument", "s");
		string arg = args.at(0)->ToString();
		result->SetBool(this->application->HasArgument(arg));
	}

	void ApplicationBinding::_GetArgumentValue(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getArgumentValue", "s");
		string arg = args.at(0)->ToString();
		string argValue = this->application->GetArgumentValue(arg);
		result->SetString(argValue);
	}

	void ApplicationBinding::_GetDependencies(const ValueList& args, KValueRef result)
	{
		result->SetList(APIBinding::DependencyVectorToKList(
			this->application->dependencies));
	}

	void ApplicationBinding::_ResolveDependencies(const ValueList& args, KValueRef result)
	{
		std::vector<SharedDependency> unresolved = this->application->ResolveDependencies();
		result->SetList(APIBinding::DependencyVectorToKList(unresolved));
	}

	void ApplicationBinding::_GetComponents(const ValueList& args, KValueRef result)
	{
		// Do not use a reference here, because we don't want to modify the
		// application's modules list.
		std::vector<SharedComponent> components = this->application->modules;

		if (!this->application->runtime.isNull())
		{
			components.push_back(this->application->runtime);
		}

		for (size_t i = 0; i < this->application->sdks.size(); i++)
		{
			components.push_back(this->application->sdks[i]);
		}
		KListRef componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetModules(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent>& components = this->application->modules;
		KListRef componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetRuntime(const ValueList& args, KValueRef result)
	{
		if (!this->application->runtime.isNull())
		{
			result->SetObject(new ComponentBinding(this->application->runtime));
		}
		else
		{
			result->SetNull();
		}
	}

	void ApplicationBinding::_GetAvailableComponents(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components);
		KListRef componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetAvailableModules(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components);
		KListRef componentList = APIBinding::ComponentVectorToKList(components, MODULE);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetAvailableRuntimes(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components);
		KListRef componentList = APIBinding::ComponentVectorToKList(components, RUNTIME);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetBundledComponents(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components, true);
		KListRef componentList = APIBinding::ComponentVectorToKList(components);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetBundledModules(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components, true);
		KListRef componentList = APIBinding::ComponentVectorToKList(components, MODULE);
		result->SetList(componentList);
	}

	void ApplicationBinding::_GetBundledRuntimes(const ValueList& args, KValueRef result)
	{
		std::vector<SharedComponent> components;
		this->application->GetAvailableComponents(components, true);
		KListRef componentList = APIBinding::ComponentVectorToKList(components, RUNTIME);
		result->SetList(componentList);
	}
}
