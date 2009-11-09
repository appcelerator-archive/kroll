/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LINUX_HOST_H
#define _LINUX_HOST_H

#include <vector>
#include <string>
#include <kroll/kroll.h>

namespace kroll
{

	class EXPORT LinuxHost : public Host
	{
	public:
		LinuxHost(int argc, const char* argv[]);
		void Exit(int returnCode);
		virtual Module* CreateModule(std::string& path);
		const char* GetPlatform();
		const char* GetModuleSuffix();
		bool IsMainThread();


	protected:
		pthread_t mainThread;
		virtual bool RunLoop();
		virtual ~LinuxHost();
	};
}

extern "C"
{
	EXPORT int Execute(int argc,const char** argv);
}


#endif
