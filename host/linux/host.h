/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LINUX_HOST_H
#define _LINUX_HOST_H

#include <vector>
#include <string>
#include <api/module.h>
#include <api/module_provider.h>
#include <api/host.h>
#include <api/binding/bound_object.h>

namespace kroll
{
	class EXPORT LinuxHost : public Host
	{
	public:
		LinuxHost(int argc, const char* argv[]);
		virtual ~LinuxHost();

		virtual int Run();
		virtual Module* CreateModule(std::string& path);
	};
}

EXPORT kroll::Host* createHost;


#endif
