/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _API_PLUGIN_H
#define _API_PLUGIN_H

#include <kroll/kroll.h>
#include "api_binding.h"

namespace kroll
{
	class KROLL_API APIModule : public Module
	{
	public:
		APIModule(Host* host, const char* path) :
			Module(host, path, STRING(MODULE_NAME), STRING(MODULE_VERSION))
		{

		}

		~APIModule()
		{

		}

		void Initialize();
		void Stop();

	protected:
		AutoPtr<APIBinding> binding;
	};
}

#endif
