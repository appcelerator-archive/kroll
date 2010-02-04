/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _JAVASCRIPT_MODULE_INSTANCE_H_
#define _JAVASCRIPT_MODULE_INSTANCE_H_

#include "javascript_api.h"

namespace kroll
{
	class KROLL_JAVASCRIPT_API JavascriptModuleInstance : public Module
	{
	public:
		JavascriptModuleInstance(Host *host, std::string path, 
			std::string dir, std::string name);
		void Initialize () {}
		void Stop();
		void Run();
		static void GarbageCollect();

	protected:
		std::string path;
		JSGlobalContextRef context;
		JSObjectRef global;
	};
}

#endif
