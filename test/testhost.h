/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TEST_HOST_H
#define _TEST_HOST_H

#include <api/module.h>
#include <api/host.h>
#include <api/binding/bound_object.h>

namespace kroll
{
	class EXPORT TestHost : public Host
	{
	public:
		TestHost(std::vector<std::string>& modules);
		virtual ~TestHost();

		virtual int Run();
		virtual Module* CreateModule(std::string& path);

	private:
		std::vector<std::string> module_paths, modules;
		std::string appConfig;
	};
}

#endif
