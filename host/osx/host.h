/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _OSX_HOST_H
#define _OSX_HOST_H

#include <api/kroll.h>
#include <api/module.h>
#include <api/host.h>
#include <api/binding/bound_object.h>

namespace kroll
{
	class EXPORT OSXHost : public Host
	{
	public:
		OSXHost(int argc, const char **argv);
		virtual ~OSXHost();

		virtual int Run();
		virtual Module* CreateModule(std::string& path);

	private:
		std::string appConfig;
	};
}

EXPORT kroll::Host* createHost;

#endif
