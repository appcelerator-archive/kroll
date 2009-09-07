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
	JavascriptModuleInstance::JavascriptModuleInstance(Host *host, std::string path, 
			std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"),
		path(path)
	{
		try
		{
			this->Load();
			this->Run();
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("Javascript");
			logger->Error("Could not execute %s because %s", path.c_str(), (*ss).c_str());
		}

	}
	void JavascriptModuleInstance::Initialize () {}
	void JavascriptModuleInstance::Destroy () {}

	void JavascriptModuleInstance::Load()
	{
		this->code = FileUtils::ReadFile(this->path);
	}

	void JavascriptModuleInstance::Run()
	{

		JSValueRef exception;
		JSGlobalContextRef context = JSGlobalContextCreate(NULL);
		JSObjectRef globalObject = JSContextGetGlobalObject(context);
		KJSUtil::RegisterGlobalContext(globalObject, context);

		/* Take some steps to insert the API into the Javascript context */
		/* Create a crazy, crunktown delegate hybrid object for Javascript */
		SharedValue globalValue = Value::NewObject(host->GetGlobalObject());

		/* convert JS API to a KJS object */
		JSValueRef jsAPI = KJSUtil::ToJSValue(globalValue, context);

		/* set the API as a property of the global object */
		JSStringRef propertyName = JSStringCreateWithUTF8CString(PRODUCT_NAME);
		JSObjectSetProperty(context, globalObject, propertyName,
		                    jsAPI, kJSPropertyAttributeNone, NULL);
		JSStringRelease(propertyName);

		/* Try to run the script */
		JSStringRef jsCode = JSStringCreateWithUTF8CString(this->code.c_str());

		/* check script syntax */
		bool syntax = JSCheckScriptSyntax(context, jsCode, NULL, 0, &exception);
		
		if (!syntax)
		{
			SharedValue e = KJSUtil::ToKrollValue(exception, context, NULL);
			JSStringRelease(jsCode);
			throw ValueException(e);
		}

		/* evaluate the script */
		JSValueRef ret = JSEvaluateScript(context, jsCode, NULL, NULL, 1, &exception);
		JSStringRelease(jsCode);
		
		if (ret == NULL)
		{
			SharedValue e = KJSUtil::ToKrollValue(exception, context, NULL);
			throw ValueException(e);
		}
		
		// null it out so we don't hold a bunch of this in memory
		this->code = "";
	}

}

