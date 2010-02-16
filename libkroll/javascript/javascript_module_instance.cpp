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

	JavaScriptModuleInstance::JavaScriptModuleInstance(Host* host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"),
		path(path),
		context(0)
	{
		this->context = KJSUtil::CreateGlobalContext();
		KJSUtil::ProtectGlobalContext(context);

		try
		{
			this->Run();
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("JavaScript");
			logger->Error("Could not execute %s because %s", path.c_str(), (*ss).c_str());
		}

		instanceContexts.push_back(this->context);
	}

	void JavaScriptModuleInstance::Stop()
	{
		KJSUtil::UnregisterGlobalContext(context);
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

		this->context = 0;
	}

	void JavaScriptModuleInstance::Run()
	{
		std::string code(FileUtils::ReadFile(this->path));

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

		KJSUtil::Evaluate(context, code.c_str());
	}

	/*static*/
	void JavaScriptModuleInstance::GarbageCollect()
	{
		for (size_t i = 0; i < instanceContexts.size(); i++)
			JSGarbageCollect(instanceContexts[i]);
	}
}

