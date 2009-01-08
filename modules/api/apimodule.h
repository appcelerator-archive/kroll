/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _API_PLUGIN_H
#define _API_PLUGIN_H

#include <api/module.h>
#include <api/host.h>
#include "api.h"
#include "apibinding.h"

namespace kroll
{
	class KROLL_API_API APIModule : public Module
	{
		KROLL_MODULE_CLASS(APIModule)

		void Test();

	protected:
		StaticBoundObject *parent;
		APIBinding *binding;
	};
}

#endif
