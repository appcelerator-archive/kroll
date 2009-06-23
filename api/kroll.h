/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
/**
 * This file is the easiest way to gain access to the full Kroll API
 * To use it, just do the following:
 * \code
 * #include <kroll/kroll.h>
 * \endcode
 */

#ifndef _KROLL_H_
#define _KROLL_H_

#include "base.h"
#include <Poco/SharedPtr.h>
#include <vector>

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
	class Value;
	class KObject;
	class KMethod;
	class KList;

	class StaticBoundObject;
	class StaticBoundMethod;
	class StaticBoundList;

	class DelegateStaticBoundObject;
	class ScopeMethodDelegate;
	class Blob;
	class ValueReleasePolicy;
	class Logger;
	class ArgList;

	typedef SharedPtr<Value, Poco::ReferenceCounter, ValueReleasePolicy> SharedValue;
	typedef SharedPtr<KObject> SharedKObject;
	typedef SharedPtr<KMethod> SharedKMethod;
	typedef SharedPtr<KList> SharedKList;

	typedef SharedPtr<std::string> SharedString;
	typedef std::vector<SharedString> StringList;
	typedef SharedPtr<StringList> SharedStringList;

	typedef ArgList ValueList;

	class Module;
	class Application;
	class KComponent;
	class Dependency;

	typedef SharedPtr<KComponent> SharedComponent;
	typedef SharedPtr<Application> SharedApplication;
	typedef SharedPtr<Dependency> SharedDependency;
}

#ifndef OS_WIN32
	#pragma GCC visibility pop
#endif

#include "utils/utils.h"
#include "logger.h"
#include "mutex.h"
#include "scoped_lock.h"

#include "binding/binding.h"
#include "module_provider.h"
#include "module.h"
#include "async_job.h"
#include "host.h"

#ifdef OS_OSX
#include "osx/osx.h"
#endif

#endif
