/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_MODULE_PROVIDER_H_
#define _KR_MODULE_PROVIDER_H_
#include <string>

namespace kroll
{
	class Module;
	class Host;

	typedef Module* ModuleCreator(Host *host);

	class EXPORT ModuleProvider
	{
	public:
		ModuleProvider() {}
		virtual ~ModuleProvider() {};
		virtual std::string GetDescription() = 0;
		virtual bool IsModule(std::string& filename) = 0;
		virtual Module* CreateModule(std::string& path) = 0;
	};
}

#endif
