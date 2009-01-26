/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KROLL_H_
#define _KROLL_H_

#include <Poco/SharedPtr.h>
#include <vector>
#include "base.h"

using Poco::SharedPtr;

#ifndef OS_WIN32
	// this is important which essentially marks all of
	// these classes below and the typedef/templates to be
	// visible outside of the library.  if you don't do this
	// you won't be able to catch exceptions of SharedValue for
	// example
	#pragma GCC visibility push(default)
#endif

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
	class DelegateStaticBoundObject;
	class ScopeMethodDelegate;

	class ValueReleasePolicy;

	typedef SharedPtr<Value, Poco::ReferenceCounter, ValueReleasePolicy> SharedValue;
	typedef SharedPtr<BoundObject> SharedBoundObject;
	typedef SharedPtr<BoundMethod> SharedBoundMethod;
	typedef SharedPtr<BoundList> SharedBoundList;

	typedef SharedPtr<std::string> SharedString;
	typedef std::vector<SharedString> StringList;
	typedef SharedPtr<StringList> SharedStringList;

	/*
	 * Type: ValueList
	 *
	 * This typdef is only used for argument lists. For
	 * a list implementation to be used as a value in the
	 * binding layer, take a look at BoundList and
	 * StaticBoundList.
	 */
	typedef std::vector<SharedValue> ValueList;


}

#ifndef OS_WIN32
	#pragma GCC visibility pop
#endif

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
