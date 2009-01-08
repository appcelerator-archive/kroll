/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _FOO_PLUGIN_H
#define _FOO_PLUGIN_H

#include <api/module.h>
#include <api/host.h>
#include "foobinding.h"

namespace kroll
{
	class FooModule : public Module
	{
		KROLL_MODULE_CLASS(FooModule)

	protected:
		FooBinding *binding;
	};
}

#endif
