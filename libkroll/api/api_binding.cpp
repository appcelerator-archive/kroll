/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "api_binding.h"
#include "application_binding.h"
#include "component_binding.h"
#include "dependency_binding.h"
#include "environment_binding.h"
#include "script_binding.h"
#include <kroll/thread_manager.h>
#include <algorithm>

using std::string;
using std::vector;
using std::pair;
using std::map;

namespace kroll
{
	APIBinding::APIBinding(Host* host) :
		KAccessorObject("API"),
		host(host),
		global(host->GetGlobalObject()),
		logger(Logger::Get("API")),
		installerThread(0),
		installerThreadAdapter(0)
	{
		this->installerThreadAdapter = new Poco::RunnableAdapter<APIBinding>(
			*this, &APIBinding::RunInstaller);

		/**
		 * @tiapi(method=True,name=API.set,since=0.2)
		 * @tiapi Set an attribute in the global object
		 * @tiarg[String, key] Key of the attribute to set
		 * @tiarg[String, value] New value of the attribute
		 */
		this->SetMethod("set", &APIBinding::_Set);

		/**
		 * @tiapi(method=True,name=API.get,since=0.2)
		 * @tiapi Get an attribute in the global object
		 * @tiarg[String, key] Key of the attribute to get
		 * @tiresult[any] Attribute at that key
		 */
		this->SetMethod("get", &APIBinding::_Get);

		/**
		 * @tiapi(method=True,name=API.addEventListener,since=0.5)
		 * @tiapi Register a root event listener
		 * @tiarg[String, eventName] The event name to listen for
		 * @tiarg[Function, callback] The callback to invoke when this message occurs
		 * @tiresult[Number] An id which represents this event listener
		 */
		this->SetMethod("addEventListener", &APIBinding::_AddEventListener);

		/**
		 * @tiapi(method=True,name=API.removeEventListener,since=0.5)
		 * @tiapi Remove a root event listener
		 * @tiarg[Number|Function, id] The id or callback of the event listener to remove
		 */
		this->SetMethod("removeEventListener", &APIBinding::_RemoveEventListener);

		/**
		 * @tiapi(method=True,name=API.fireEvent,since=0.5)
		 * @tiapi Fire the event with a given name
		 * @tiarg[String|Object, event] The name of the event to fire or the event itself
		 */
		this->SetMethod("fireEvent", &APIBinding::_FireEvent);

		/**
		 * @tiapi(method=True,name=API.runOnMainThread,since=0.5)
		 * @tiapi Execute the method on the main thread
		 * @tiarg[Function, method] The method to execute
		 * @tiarg[any, ...] A variable-length list of arguments to pass to the method
		 * @tiresult[any] The return value of the method
		 */
		this->SetMethod("runOnMainThread", &APIBinding::_RunOnMainThread);

		/**
		 * @tiapi(method=True,name=API.runOnMainThreadAsync,since=0.5)
		 * @tiapi Execute the method asynchronously on the main thread
		 * @tiarg[Function, method] The method to execute
		 * @tiarg[any, ...] A variable-length list of arguments to pass to the method
		 */
		this->SetMethod("runOnMainThreadAsync", &APIBinding::_RunOnMainThreadAsync);

		/**
		 * @tiapi(method=True,name=API.getApplication,since=0.2)
		 * @tiapi Get the currently running application
		 * @tiresult[API.Application] an API.Application that is the running application
		 */
		this->SetMethod("getApplication", &APIBinding::_GetApplication);

		/**
		 * @tiapi(method=True,name=API.getInstalledComponents,since=0.4)
		 * @tiapi Get a list of the currently installed Kroll components
		 * @tiresult[Array<API.Component>] a list of API.Component of installed components of all types
		 */
		this->SetMethod("getInstalledComponents", &APIBinding::_GetInstalledComponents);

		/**
		 * @tiapi(method=True,name=API.getInstalledSDKs,since=0.4)
		 * @tiapi Get a list of the currently installed Kroll SDK components
		 * @tiresult[Array<API.Component>] a list of API.Component of installed SDK components
		 */
		this->SetMethod("getInstalledSDKs", &APIBinding::_GetInstalledSDKs);

		/**
		 * @tiapi(method=True,name=API.getInstalledMobileSDKs,since=0.4)
		 * @tiapi Get a list of the currently installed Kroll Mobile SDK components
		 * @tiresult[Array<API.Component>] a list of API.Component of installed Mobile SDK components
		 */
		this->SetMethod("getInstalledMobileSDKs", &APIBinding::_GetInstalledMobileSDKs);

		/**
		 * @tiapi(method=True,name=API.getInstalledModules,since=0.4)
		 * @tiapi Get a list of the currently installed Kroll module components
		 * @tiresult[Array<API.Component>] a list of API.Component of installed module components
		 */
		this->SetMethod("getInstalledModules", &APIBinding::_GetInstalledModules);

		/**
		 * @tiapi(method=True,name=API.getInstalledRuntimes,since=0.4)
		 * @tiapi Get a list of the currently installed Kroll runtime components
		 * @tiresult[Array<API.Component>] a list of API.Component of installed runtime components
		 */
		this->SetMethod("getInstalledRuntimes", &APIBinding::_GetInstalledRuntimes);

		/**
		 * @tiapi(method=True,name=API.getComponentSearchPaths,since=0.4)
		 * @tiapi Get a list of the paths on which Kroll searches for installed
		 * @tiapi components. This does not include paths of bundled components.
		 * @tiresult[Array<API.Component>] a list of string of component search paths
		 */
		this->SetMethod("getComponentSearchPaths", &APIBinding::_GetComponentSearchPaths);

		/**
		 * @tiapi(method=True,name=API.readApplicationManifest,since=0.4)
		 * @tiapi Read an application manifest at a given path
		 * @tiarg[String, manifestPath] the path to the manifest to read
		 * @tiarg[String, applicationPath,optional=True] an optional application path override
		 * @tiresult[API.Application] an API.Application which represents the application with given manifest
		 */
		this->SetMethod("readApplicationManifest", &APIBinding::_ReadApplicationManifest);

		/**
		 * @tiapi(method=True,name=API.createDependency,since=0.4)
		 * @tiapi A constructor for dependency objects
		 * @tiarg[Number, type] the type of this dependency (eg API.MODULE)
		 * @tiarg[String, name] the name of this dependency
		 * @tiarg[String, version] the version requirement for this dependency
		 * @tiarg[Number, requirement, optional=true] the requirement for this dependency
		 * @tiresult[API.Dependency] A new Dependency.
		 */
		this->SetMethod("createDependency", &APIBinding::_CreateDependency);

		/**
		 * @tiapi(method=True,name=API.installDependencies,since=0.4)
		 * @tiapi Invoke the installer to find and install a list of dependencies
		 * @tiarg[Array<API.Dependency>, dependencies] a list of API.Dependency to find and install
		 * @tiarg[method, callback] a callback to invoke when the installer is finished (may finish unsuccessfully)
		 */
		this->SetMethod("installDependencies", &APIBinding::_InstallDependencies);

		this->SetMethod("componentGUIDToComponentType", &APIBinding::_ComponentGUIDToComponentType);

		/**
		 * @tiapi(method=True,name=API.getEnvironment,since=0.5)
		 * @tiapi Get the system environment
		 * @tiresult[API.Environment] An object representing the current environment. 
		 * @tiresult Setting an environment variable is the same as setting a property.
		 * @tiresult e.g env["HOME"] = "/myhome"
		 */
		this->SetMethod("getEnvironment", &APIBinding::_GetEnvironment);

		/**
		 * @tiapi(method=True,name=API.createKObject,since=0.5) Create a Kroll object.
		 * @tiarg[Object, toWrap, optional=true] An object to wrap in a new KObject.
		 * @tiresult[Object] A new KObject.
		 */
		this->SetMethod("createKObject", &APIBinding::_CreateKObject);

		/**
		 * @tiapi(method=True,name=API.createKMethod,since=0.5) create a Kroll method.
		 * @tiarg[Function, toWrap, optional=true] A function to wrap in a new KMethod
		 * @tiresult[Function] A new KMethod.
		 */
		this->SetMethod("createKMethod", &APIBinding::_CreateKMethod);
		
		/**
		 * @tiapi(method=True,name=API.createKList,since=0.5) create a Kroll list.
		 * @tiarg[Array, toWrap, optional=true] A function to wrap in a new KMethod
		 * @tiresult[Array] A new KList.
		 */
		this->SetMethod("createKList", &APIBinding::_CreateKList);

		/**
		 * @tiapi(method=True,name=API.createBytes,since=0.9) Create a Kroll Bytes object.
		 * @tiarg[String, contents, optional=true] The contents of the new Bytes.
		 * @tiarg The blob will be empty if none are given.
		 * @tiresult[Bytes] A new Bytes.
		 */
		this->SetMethod("createBytes", &APIBinding::_CreateBytes);
		this->SetMethod("createBlob", &APIBinding::_CreateBytes);

		/**
		 * @tiapi(method=True,name=API.log,since=0.2)
		 * @tiapi Log a statement with a given severity
		 * @tiarg[Number, type] the severity of this log statement
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("log", &APIBinding::_Log);

		/**
		 * @tiapi(method=True,name=API.trace,since=0.4)
		 * @tiapi Log a statement with TRACE severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("trace", &APIBinding::_LogTrace);

		/**
		 * @tiapi(method=True,name=API.debug,since=0.4)
		 * @tiapi Log a statement with DEBUG severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("debug", &APIBinding::_LogDebug);

		/**
		 * @tiapi(method=True,name=API.info,since=0.4)
		 * @tiapi Log a statement with INFO severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("info", &APIBinding::_LogInfo);

		/**
		 * @tiapi(method=True,name=API.notice,since=0.4)
		 * @tiapi Log a statement with NOTICE severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("notice", &APIBinding::_LogNotice);

		/**
		 * @tiapi(method=True,name=API.warn,since=0.4)
		 * @tiapi Log a statement with WARN severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("warn", &APIBinding::_LogWarn);

		/**
		 * @tiapi(method=True,name=API.error,since=0.4)
		 * @tiapi Log a statement with ERROR severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("error", &APIBinding::_LogError);

		/**
		 * @tiapi(method=True,name=API.critical,since=0.4)
		 * @tiapi Log a statement with CRITICAL severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("critical", &APIBinding::_LogCritical);

		/**
		 * @tiapi(method=True,name=API.fatal,since=0.4)
		 * @tiapi Log a statement with FATAL severity
		 * @tiarg[String, statement] the statement to log
		 */
		this->SetMethod("fatal", &APIBinding::_LogFatal);

		/**
		 * @tiapi(method=True,name=API.setLogLevel,since=0.5)
		 * @tiapi Set the log level of the root logger
		 * @tiarg[Number, level] the threshold of severity to log
		 */
		this->SetMethod("setLogLevel", &APIBinding::_SetLogLevel);

		/**
		 * @tiapi(method=True,name=API.getLogLevel,since=0.5)
		 * @tiapi Get the log level of the root logger
		 * @tiresult[Number] the threshold of severity to log
		 */
		this->SetMethod("getLogLevel", &APIBinding::_GetLogLevel);
	
		/**
		 * @tiapi(method=True,name=API.print,since=0.6)
		 * @tiapi print a raw string to stdout (no newlines are appended)
		 * @tiarg[Any, data] data to print
		 */
		this->SetMethod("print", &APIBinding::_Print);
	
		// These are properties for log severity levels

		/**
		 * @tiapi(property=True,name=API.TRACE,since=0.4)
		 * @tiapi a constant representing TRACE severity
		 */
		this->Set("TRACE", Value::NewInt(Logger::LTRACE));

		/**
		 * @tiapi(property=True,name=API.DEBUG,since=0.4)
		 * @tiapi a constant representing DEBUG severity
		 */
		this->Set("DEBUG", Value::NewInt(Logger::LDEBUG));

		/**
		 * @tiapi(property=True,name=API.INFO,since=0.4)
		 * @tiapi a constant representing INFO severity
		 */
		this->Set("INFO", Value::NewInt(Logger::LINFO));

		/**
		 * @tiapi(property=True,name=API.NOTICE,since=0.4)
		 * @tiapi a constant representing NOTICE severity
		 */
		this->Set("NOTICE", Value::NewInt(Logger::LNOTICE));

		/**
		 * @tiapi(property=True,name=API.WARN,since=0.4)
		 * @tiapi a constant representing WARN severity
		 */
		this->Set("WARN", Value::NewInt(Logger::LWARN));

		/**
		 * @tiapi(property=True,name=API.ERROR,since=0.4)
		 * @tiapi a constant representing ERROR severity
		 */
		this->Set("ERROR", Value::NewInt(Logger::LERROR));

		/**
		 * @tiapi(property=True,name=API.CRITICAL,since=0.4)
		 * @tiapi a constant representing CRITICAL severity
		 */
		this->Set("CRITICAL", Value::NewInt(Logger::LCRITICAL));

		/**
		 * @tiapi(property=True,name=API.FATAL,since=0.4)
		 * @tiapi a constant representing FATAL severity
		 */
		this->Set("FATAL", Value::NewInt(Logger::LFATAL));

		// These are properties for dependencies

		/**
		 * @tiapi(property=True,name=API.EQ,since=0.4)
		 * @tiapi a constant representing an equality dependency
		 */
		this->Set("EQ", Value::NewInt(Dependency::EQ));

		/**
		 * @tiapi(property=True,name=API.LT,since=0.4)
		 * @tiapi a constant representing a less-than dependency
		 */
		this->Set("LT", Value::NewInt(Dependency::LT));

		/**
		 * @tiapi(property=True,name=API.LTE,since=0.4)
		 * @tiapi a constant representing a less-than-or-equal dependency
		 */
		this->Set("LTE", Value::NewInt(Dependency::LTE));

		/**
		 * @tiapi(property=True,name=API.GT,since=0.4)
		 * @tiapi a constant representing a greater-than dependency
		 */
		this->Set("GT", Value::NewInt(Dependency::GT));

		/**
		 * @tiapi(property=True,name=API.GTE,since=0.4)
		 * @tiapi a constant representing a greather-than-or-equal dependency
		 */
		this->Set("GTE", Value::NewInt(Dependency::GTE));

		// Component types

		/**
		 * @tiapi(property=True,name=API.MODULE,since=0.4)
		 * @tiapi a constant representing a module component type
		 */
		this->Set("MODULE", Value::NewInt(MODULE));

		/**
		 * @tiapi(property=True,name=API.RUNTIME,since=0.4)
		 * @tiapi a constant representing a runtime component type
		 */
		this->Set("RUNTIME", Value::NewInt(RUNTIME));

		/**
		 * @tiapi(property=True,name=API.SDK,since=0.4)
		 * @tiapi a constant representing an SDK component type
		 */
		this->Set("SDK", Value::NewInt(SDK));

		/**
		 * @tiapi(property=True,name=API.MOBILESDK,since=0.4)
		 * @tiapi a constant representing a mobile SDK component type
		 */
		this->Set("MOBILESDK", Value::NewInt(MOBILESDK));

		/**
		 * @tiapi(property=True,name=API.APP_UPDATE,since=0.4)
		 * @tiapi a constant representing an app update component type
		 */
		this->Set("APP_UPDATE", Value::NewInt(APP_UPDATE));

		/**
		 * @tiapi(property=True,name=API.UNKNOWN,since=0.4)
		 * @tiapi a constant representing an UNKNOWN component type
		 */
		this->Set("UNKNOWN", Value::NewInt(UNKNOWN));
		
		this->Set("Script", Value::NewObject(new ScriptBinding()));
	}

	APIBinding::~APIBinding()
	{
	}

	void APIBinding::_Set(const ValueList& args, KValueRef result)
	{
		const char *key = args.at(0)->ToString();
		string s = key;
		KValueRef value = args.at(1);
		string::size_type pos = s.find_first_of(".");

		if (pos==string::npos)
		{
			this->Set(key, value);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			global->SetNS(key, value);
		}
	}

	Logger::Level APIBinding::ValueToLevel(KValueRef v)
	{
		if (v->IsString())
		{
			string levelString = v->ToString();
			return Logger::GetLevel(levelString);
		}
		else if (v->IsObject())
		{
			SharedString ss = v->ToObject()->DisplayString();
			return Logger::GetLevel(*ss);
		}
		else if (v->IsNumber())
		{
			return (Logger::Level) v->ToInt();
		}
		else // return the appropriate default
		{
			string levelString = "";
			return Logger::GetLevel(levelString);
		}
	}

	void APIBinding::_Get(const ValueList& args, KValueRef result)
	{
		string s = args.at(0)->ToString();
		const char *key = s.c_str();
		KValueRef r = 0;
		string::size_type pos = s.find_first_of(".");

		if (pos==string::npos)
		{
			r = this->Get(key);
		}
		else
		{
			// if we have a period, make it relative to the
			// global scope such that <module>.<key> would resolve
			// to the 'module' with key named 'key'
			r = global->GetNS(key);
		}
		result->SetValue(r);
	}

	void APIBinding::_SetLogLevel(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setLogLevel", "s|n");
		Logger::GetRootLogger()->SetLevel(ValueToLevel(args.at(0)));
	}

	void APIBinding::_GetLogLevel(const ValueList& args, KValueRef result)
	{
		result->SetInt(Logger::GetRootLogger()->GetLevel());
	}

	void APIBinding::_Print(const ValueList& args, KValueRef result)
	{
		for (size_t c=0; c < args.size(); c++)
		{
			KValueRef arg = args.at(c);
			if (arg->IsString())
			{
				const char *s = arg->ToString();
				std::cout << s;
			}
			else
			{
				SharedString ss = arg->DisplayString();
				std::cout << *ss;
			}
		}
		std::cout.flush();
	}
	
	void APIBinding::_LogTrace(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LTRACE, arg1);
	}
	void APIBinding::_LogDebug(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LDEBUG, arg1);
	}
	void APIBinding::_LogInfo(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LINFO, arg1);
	}
	void APIBinding::_LogNotice(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LNOTICE, arg1);
	}
	void APIBinding::_LogWarn(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LWARN, arg1);
	}
	void APIBinding::_LogError(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LERROR, arg1);
	}
	void APIBinding::_LogCritical(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LCRITICAL, arg1);
	}
	void APIBinding::_LogFatal(const ValueList& args, KValueRef result)
	{
		KValueRef arg1 = args.at(0);
		this->Log(Logger::LFATAL, arg1);
	}

	void APIBinding::_Log(const ValueList& args, KValueRef result)
	{
		if (args.size() == 1)
		{
			KValueRef v = args.at(0);
			this->Log(Logger::LINFO, v);
		}
		else if (args.size() == 2)
		{
			KValueRef arg1 = args.at(0);
			
			KValueRef v = args.at(1);
			this->Log(ValueToLevel(arg1), v);
		}
	}

	void APIBinding::_AddEventListener(const ValueList& args, KValueRef result)
	{
		GlobalObject::GetInstance()->_AddEventListener(args, result);
	}

	void APIBinding::_RemoveEventListener(const ValueList& args, KValueRef result)
	{
		GlobalObject::GetInstance()->_RemoveEventListener(args, result);
	}

	void APIBinding::_FireEvent(const ValueList& args, KValueRef result)
	{
		args.VerifyException("fireEvent", "s|o");
		if (args.at(0)->IsString())
		{
			std::string eventName = args.GetString(0);
			GlobalObject::GetInstance()->FireEvent(eventName);
		}
		else if (args.at(0)->IsObject())
		{
			AutoPtr<Event> event = args.GetObject(0).cast<Event>();
			if (!event.isNull())
				GlobalObject::GetInstance()->FireEvent(event);
		}
	}

	void APIBinding::_RunOnMainThread(const ValueList& args, KValueRef result)
	{
		if (!args.at(0)->IsMethod())
		{
			throw ValueException::FromString(
				"First argument to runOnMainThread was not a function");

		}
		else
		{
			ValueList outArgs;
			for (size_t i = 1; i < args.size(); i++)
				outArgs.push_back(args.at(i));

			result->SetValue(RunOnMainThread(args.GetMethod(0), outArgs));
		}
	}

	void APIBinding::_RunOnMainThreadAsync(const ValueList& args, KValueRef result)
	{
		if (!args.at(0)->IsMethod())
		{
			throw ValueException::FromString(
				"First argument to runOnMainThread was not a function");

		}
		else
		{
			ValueList outArgs;
			for (size_t i = 1; i < args.size(); i++)
				outArgs.push_back(args.at(i));

			RunOnMainThread(args.GetMethod(0), outArgs, false);
		}
	}

	//---------------- IMPLEMENTATION METHODS
	void APIBinding::Log(int severity, KValueRef value)
	{
		// optimize these calls since they're called a lot
		if (false == logger->IsEnabled((Logger::Level)severity))
		{
			return;
		}

		if (value->IsString())
		{
			string message = value->ToString();
			logger->Log((Logger::Level) severity, message);
		}
		else
		{
			SharedString message = value->DisplayString();
			logger->Log((Logger::Level) severity, *message);
		}
	}

	void APIBinding::_GetApplication(const ValueList& args, KValueRef result)
	{
		KObjectRef app = new ApplicationBinding(host->GetApplication(), true);
		result->SetObject(app);
	}

	void APIBinding::_GetInstalledComponentsImpl(
		KComponentType type, const ValueList& args, KValueRef result)
	{
		bool force = args.GetBool(0, false);
		vector<SharedComponent>& components = BootUtils::GetInstalledComponents(force);
		KListRef componentList = ComponentVectorToKList(components, type);
		result->SetList(componentList);
	}

	void APIBinding::_GetInstalledComponents(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getInstalledComponents", "?b");
		_GetInstalledComponentsImpl(UNKNOWN, args, result);
	}

	void APIBinding::_GetInstalledModules(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getInstalledModules", "?b");
		_GetInstalledComponentsImpl(MODULE, args, result);
	}

	void APIBinding::_GetInstalledRuntimes(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getInstalledRuntimes", "?b");
		_GetInstalledComponentsImpl(RUNTIME, args, result);
	}

	void APIBinding::_GetInstalledSDKs(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getInstalledSDKs", "?b");
		_GetInstalledComponentsImpl(SDK, args, result);
	}

	void APIBinding::_GetInstalledMobileSDKs(const ValueList& args, KValueRef result)
	{
		args.VerifyException("getInstalledMobileSDKs", "?b");
		_GetInstalledComponentsImpl(MOBILESDK, args, result);
	}

	void APIBinding::_GetComponentSearchPaths(const ValueList& args, KValueRef result)
	{
		vector<string>& paths = BootUtils::GetComponentSearchPaths();
		KListRef pathList = StaticBoundList::FromStringVector(paths);
		result->SetList(pathList);
	}

	void APIBinding::_ReadApplicationManifest(const ValueList& args, KValueRef result)
	{
		args.VerifyException("readApplicationManifest", "s,?s");
		string manifestPath = args.at(0)->ToString();
		string appPath = args.GetString(1, FileUtils::Dirname(manifestPath));

		SharedApplication app = Application::NewApplication(manifestPath, appPath);
		if (!app.isNull())
		{
			result->SetObject(new ApplicationBinding(app));
		}
		else
		{
			result->SetNull();
		}
	}

	void APIBinding::_CreateDependency(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createDepenendency", "i,s,s,?i");
		int type = args.GetInt(0, UNKNOWN);
		string name = args.GetString(1);
		string version = args.GetString(2);
		int requirement = (int) args.GetNumber(3, Dependency::EQ);

		if (type != MODULE && type != RUNTIME
			&& type != SDK && type != MOBILESDK
			&& type != APP_UPDATE)
		{
			throw ValueException::FromString(
				"Tried to create a dependency with an unknown dependency type");
		}
		else if (requirement != Dependency::EQ
			&& requirement != Dependency::GT
			&& requirement != Dependency::LT
			&& requirement != Dependency::GTE
			&& requirement != Dependency::LTE)
		{
			throw ValueException::FromString(
				"Tried to create a dependency with an unknown requirement type");
		}
		else
		{
			SharedDependency d = Dependency::NewDependencyFromValues(
				static_cast<KComponentType>(type), name, version);
			KObjectRef dBinding = new DependencyBinding(d);
			result->SetObject(dBinding);
		}
	}

	void APIBinding::_InstallDependencies(const ValueList& args, KValueRef result)
	{
		args.VerifyException("installDependencies", "l,m");
		KListRef dependenciesList = args.GetList(0);
		KMethodRef callback = args.GetMethod(1, 0);
		vector<SharedDependency> dependencies;

		for (unsigned int i = 0; i < dependenciesList->Size(); i++)
		{
			if (!dependenciesList->At(i)->IsObject())
			{
				continue;
			}

			AutoPtr<DependencyBinding> d =
				dependenciesList->At(i)->ToObject().cast<DependencyBinding>();
			if (!d.isNull())
			{
				dependencies.push_back(d->GetDependency());
			}
		}

		if (dependencies.size() > 0)
		{
			if (!this->installerMutex.tryLock())
			{
				throw ValueException::FromString(
					"Tried to launch more than one instance of the installer");
			}

			if (this->installerThread)
			{
				delete this->installerThread;
			}
			this->installerDependencies = dependencies;
			this->installerCallback = callback;
			this->installerThread = new Poco::Thread();
			this->installerThread->start(*this->installerThreadAdapter);
		}
	}
	
	void APIBinding::_ComponentGUIDToComponentType(const ValueList& args, KValueRef result)
	{
		std::string type = args.at(0)->ToString();
		if (type == RUNTIME_UUID)
		{
			result->SetInt(RUNTIME);
		}
		else if (type == MODULE_UUID)
		{
			result->SetInt(MODULE);
		}
		else if (type == SDK_UUID)
		{
			result->SetInt(SDK);
		}
		else if (type == MOBILESDK_UUID)
		{
			result->SetInt(MOBILESDK);
		}
		else if (type == APP_UPDATE_UUID)
		{
			result->SetInt(APP_UPDATE);
		}
		else
		{
			result->SetInt(UNKNOWN);
		}
	}
	
	void APIBinding::_GetEnvironment(const ValueList& args, KValueRef result)
	{
		AutoPtr<EnvironmentBinding> env = new EnvironmentBinding();
		result->SetObject(env);
	}
	
	void APIBinding::RunInstaller()
	{
		START_KROLL_THREAD;

		SharedApplication app = host->GetApplication();
		BootUtils::RunInstaller(
			this->installerDependencies, app, "", app->runtime->path, true);

		if (!this->installerCallback.isNull())
		{
			RunOnMainThread(this->installerCallback, ValueList(), false);
		}
		this->installerMutex.unlock();

		END_KROLL_THREAD;
	}

	KListRef APIBinding::ComponentVectorToKList(
		vector<SharedComponent>& components,
		KComponentType filter)
	{
		KListRef componentList = new StaticBoundList();
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent c = *i++;
			if (filter == UNKNOWN || filter == c->type)
			{
				KValueRef cValue = Value::NewObject(new ComponentBinding(c));
				componentList->Append(cValue);
			}
		}

		return componentList;
	}

	KListRef APIBinding::DependencyVectorToKList(std::vector<SharedDependency>& deps)
	{
		KListRef dependencyList = new StaticBoundList();
		std::vector<SharedDependency>::iterator i = deps.begin();
		while (i != deps.end())
		{
			KValueRef dValue = Value::NewObject(new DependencyBinding(*i++));
			dependencyList->Append(dValue);
		}
		return dependencyList;
	}

	KListRef APIBinding::ManifestToKList(vector<pair<string, string> >& manifest)
	{
		KListRef list = new StaticBoundList();
		vector<pair<string, string> >::iterator i = manifest.begin();
		while (i != manifest.end())
		{
			KListRef entry = new StaticBoundList();
			entry->Append(Value::NewString(i->first));
			entry->Append(Value::NewString(i->second));
			list->Append(Value::NewList(entry));
			*i++;
		}
		return list;
	}

	void APIBinding::_CreateKObject(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createKObject", "?o");
		if (args.size() <= 0)
		{
			result->SetObject(new StaticBoundObject());
		}
		else
		{
			KObjectRef wrapped = args.GetObject(0);
			result->SetObject(new KObjectWrapper(wrapped));
		}
	}

	void APIBinding::_CreateKMethod(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createKMethod", "m");
		KMethodRef wrapped = args.GetMethod(0);
		result->SetMethod(new KMethodWrapper(args.GetMethod(0)));
	}

	void APIBinding::_CreateKList(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createKList", "?l");
		if (args.size() <= 0)
		{
			result->SetList(new StaticBoundList());
		}
		else
		{
			KListRef wrapped = args.GetList(0);
			result->SetList(new KListWrapper(wrapped));
		}
	}

	static void GetBytes(KValueRef value, std::vector<BytesRef>& blobs)
	{
		if (value->IsObject())
		{
			blobs.push_back(value->ToObject().cast<Bytes>());
		}
		else if (value->IsString())
		{
			blobs.push_back(new Bytes(value->ToString()));
		}
		else if (value->IsList())
		{
			KListRef list = value->ToList();
			for (size_t j = 0; j < list->Size(); j++)
			{
				GetBytes(list->At(j), blobs);
			}
		}
		else if (value->IsNumber())
		{
			blobs.push_back(new Bytes(value->ToInt()));
		}
	}
	
	void APIBinding::_CreateBytes(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createBytes", "?s|o|l|i");
		std::vector<BytesRef> blobs;
		for (size_t i = 0; i < args.size(); i++)
		{
			GetBytes(args.at(i), blobs);
		}
		result->SetObject(Bytes::GlobBytes(blobs));
	}

	KObjectWrapper::KObjectWrapper(KObjectRef object) :
		object(object)
	{
	}

	void KObjectWrapper::Set(const char *name, KValueRef value)
	{
		object->Set(name, value);
	}

	KValueRef KObjectWrapper::Get(const char *name)
	{
		return object->Get(name);
	}

	bool KObjectWrapper::HasProperty(const char *name)
	{
		return object->HasProperty(name);	
	}
	
	SharedStringList KObjectWrapper::GetPropertyNames()
	{
		return object->GetPropertyNames();
	}

	SharedString KObjectWrapper::DisplayString(int levels)
	{
		return object->DisplayString(levels);
	}

	bool KObjectWrapper::Equals(KObjectRef other)
	{
		return object->Equals(other);	
	}
	
	KMethodWrapper::KMethodWrapper(KMethodRef method) :
		method(method)
	{
	}

	KValueRef KMethodWrapper::Call(const ValueList& args)
	{
		return method->Call(args);
	}

	void KMethodWrapper::Set(const char *name, KValueRef value)
	{
		method->Set(name, value);
	}

	KValueRef KMethodWrapper::Get(const char *name)
	{
		return method->Get(name);
	}

	bool KMethodWrapper::HasProperty(const char *name)
	{
		return method->HasProperty(name);
	}
	
	SharedStringList KMethodWrapper::GetPropertyNames()
	{
		return method->GetPropertyNames();
	}

	SharedString KMethodWrapper::DisplayString(int levels)
	{
		return method->DisplayString(levels);
	}
	
	bool KMethodWrapper::Equals(KObjectRef other)
	{
		return method->Equals(other);	
	}

	KListWrapper::KListWrapper(KListRef list) :
		list(list)
	{
	}

	void KListWrapper::Append(KValueRef value)
	{
		list->Append(value);
	}

	unsigned int KListWrapper::Size()
	{
		return list->Size();
	}

	KValueRef KListWrapper::At(unsigned int index)
	{
		return list->At(index);
	}

	void KListWrapper::SetAt(unsigned int index, KValueRef value)
	{
		list->SetAt(index, value);
	}

	bool KListWrapper::Remove(unsigned int index)
	{
		return list->Remove(index);
	}

	void KListWrapper::Set(const char *name, KValueRef value)
	{
		list->Set(name, value);
	}

	KValueRef KListWrapper::Get(const char *name)
	{
		return list->Get(name);
	}

	bool KListWrapper::HasProperty(const char *name)
	{
		return list->HasProperty(name);
	}
	
	SharedStringList KListWrapper::GetPropertyNames()
	{
		return list->GetPropertyNames();
	}

	SharedString KListWrapper::DisplayString(int levels)
	{
		return list->DisplayString(levels);
	}
	
	bool KListWrapper::Equals(KObjectRef other)
	{
		return list->Equals(other);	
	}

}
