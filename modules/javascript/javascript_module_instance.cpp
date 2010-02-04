/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"
#include <iostream>
#include <fstream>
#include <string>

namespace kroll
{
	static std::vector<JSGlobalContextRef> instanceContexts;

	JavascriptModuleInstance::JavascriptModuleInstance(Host* host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"),
		path(path)
	{
		this->context = JSGlobalContextCreate(0);
		this->global = JSContextGetGlobalObject(context);
		KJSUtil::RegisterGlobalContext(global, context);
		KJSUtil::ProtectGlobalContext(context);

		try
		{
			this->Run();
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("Javascript");
			logger->Error("Could not execute %s because %s", path.c_str(), (*ss).c_str());
		}

		instanceContexts.push_back(this->context);
	}

	void JavascriptModuleInstance::Stop()
	{
		KJSUtil::UnregisterGlobalContext(global);
		KJSUtil::UnprotectGlobalContext(context);

		std::vector<JSGlobalContextRef>::iterator i = instanceContexts.begin();
		while (i != instanceContexts.end())
		{
			if (*i == this->context)
			{
				instanceContexts.erase(i);
				break;
			}
			i++;
		}

		this->global = 0;
		this->context = 0;
	}

	void JavascriptModuleInstance::Run()
	{
		std::string code(FileUtils::ReadFile(this->path));

		// Insert the global object into this script's context.
		KValueRef globalValue = Value::NewObject(host->GetGlobalObject());
		JSValueRef jsAPI = KJSUtil::ToJSValue(globalValue, context);
		JSStringRef propertyName = JSStringCreateWithUTF8CString(PRODUCT_NAME);
		JSObjectSetProperty(context, global, propertyName, jsAPI,
			kJSPropertyAttributeNone, NULL);
		JSStringRelease(propertyName);

		// Check the script's syntax.
		JSValueRef exception;
		JSStringRef jsCode = JSStringCreateWithUTF8CString(code.c_str());
		bool syntax = JSCheckScriptSyntax(context, jsCode, NULL, 0, &exception);
		if (!syntax)
		{
			KValueRef e = KJSUtil::ToKrollValue(exception, context, NULL);
			JSStringRelease(jsCode);
			throw ValueException(e);
		}

		// Run the script.
		JSValueRef ret = JSEvaluateScript(context, jsCode, NULL, NULL, 1, &exception);
		JSStringRelease(jsCode);

		if (ret == NULL)
		{
			KValueRef e = KJSUtil::ToKrollValue(exception, context, NULL);
			throw ValueException(e);
		}
	}

	/*static*/
	void JavascriptModuleInstance::GarbageCollect()
	{
		for (size_t i = 0; i < instanceContexts.size(); i++)
			JSGarbageCollect(instanceContexts[i]);
	}
}

