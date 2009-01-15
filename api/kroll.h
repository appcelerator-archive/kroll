/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KROLL_H_
#define _KROLL_H_

namespace kroll
{
	class ScopedDereferencer;
	class Value;
	class BoundObject;
	class StaticBoundObject;
	class BoundMethod;
	class StaticBoundMethod;
	class BoundList;
	class StaticBoundList;
	class ScopeMethodDelegate;
}

#include "base.h"
#include "file_utils.h"
#include "mutex.h"
#include "scoped_lock.h"
#include "ref_counted.h"
#include "scoped_ref_counted.h"
#include "module_provider.h"
#include "module.h"
#include "host.h"
#include "scoped_dereferencer.h"
#include "binding/binding.h"

#endif
