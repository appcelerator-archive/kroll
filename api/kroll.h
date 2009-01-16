/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KROLL_H_
#define _KROLL_H_

#include <Poco/SharedPtr.h>
#include <vector>

using Poco::SharedPtr;

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

	typedef SharedPtr<Value> SharedValue;
	typedef SharedPtr<BoundObject> SharedBoundObject;
	typedef SharedPtr<BoundMethod> SharedBoundMethod;
	typedef SharedPtr<BoundList> SharedBoundList;

	/*
		Type: ValueList
	
	  This typdef is only used for argument lists. For
	  a list implementation to be used as a value in the
	  binding layer, take a look at BoundList and
	  StaticBoundList.
	 */
	typedef std::vector<SharedValue> ValueList;


}

#include "base.h"
#include "ref_counted.h"
#include "file_utils.h"
#include "scoped_ref_counted.h"
#include "scoped_dereferencer.h"
#include "mutex.h"
#include "scoped_lock.h"

#include "binding/binding.h"
#include "module_provider.h"
#include "module.h"
#include "host.h"

#endif
