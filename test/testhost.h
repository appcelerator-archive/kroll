/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TEST_HOST_H
#define _TEST_HOST_H

#include <api/kroll.h>

namespace kroll
{
	class EXPORT TestHost : public Host
	{
	public:
		TestHost(std::vector<std::string> module_paths);
		virtual ~TestHost();
		void TestAll();
		virtual Module* CreateModule(std::string& path);
		SharedValue InvokeMethodOnMainThread(SharedKMethod method,
		                                     const ValueList& args,
											 bool waitForCompletion);

	protected:
		virtual bool RunLoop();
		virtual bool Start();
		
	private:
		std::vector<std::string> module_paths;
		std::vector<Module*> test_modules;
		std::string appConfig;
	};
}

#endif
