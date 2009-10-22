/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */ 
#ifndef _DEPENDENCY_BINDING_H_
#define _DEPENDENCY_BINDING_H_

#include <kroll/kroll.h>
#include <map>
#include <vector>
#include <string>

namespace kroll
{
	class DependencyBinding : public KAccessorObject
	{
		public:
		DependencyBinding(SharedDependency dependency);
		SharedDependency GetDependency();

		private:
		SharedDependency dependency;

		void _GetType(const ValueList& args, KValueRef value);
		void _GetName(const ValueList& args, KValueRef value);;
		void _GetVersion(const ValueList& args, KValueRef value);
		void _GetRequirement(const ValueList& args, KValueRef value);
	};
}

#endif
