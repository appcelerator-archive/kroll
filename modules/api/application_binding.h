/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _APPLICATION_BINDING_H_
#define _APPLICATION_BINDING_H_

#include <kroll/kroll.h>
#include <map>
#include <vector>
#include <string>

namespace kroll
{
	class ApplicationBinding : public KAccessorObject
	{
		public:
		ApplicationBinding(SharedApplication application, bool current = false);

		private:
		SharedApplication application;
		bool current;

		void _GetID(const ValueList& args, SharedValue value);
		void _GetGUID(const ValueList& args, SharedValue value);
		void _GetName(const ValueList& args, SharedValue result);
		void _GetVersion(const ValueList& args, SharedValue value);
		void _GetPath(const ValueList& args, SharedValue value);
		void _GetExecutablePath(const ValueList& args, SharedValue value);
		void _GetResourcesPath(const ValueList& args, SharedValue value);
		void _GetDataPath(const ValueList& args, SharedValue value);
		void _GetManifestPath(const ValueList& args, SharedValue value);
		void _GetManifest(const ValueList& args, SharedValue value);
		void _GetProperties(const ValueList& args, SharedValue value);
		void _IsCurrent(const ValueList& args, SharedValue value);
		void _GetPID(const ValueList& args, SharedValue value);
		void _GetArguments(const ValueList& args, SharedValue value);
		void _HasArgument(const ValueList& args, SharedValue value);
		void _GetArgumentValue(const ValueList& args, SharedValue value);
		void _GetDependencies(const ValueList& args, SharedValue value);
		void _ResolveDependencies(const ValueList& args, SharedValue value);
		void _GetComponents(const ValueList& args, SharedValue value);
		void _GetModules(const ValueList& args, SharedValue value);
		void _GetRuntime(const ValueList& args, SharedValue value);
		void _GetAvailableComponents(const ValueList& args, SharedValue value);
		void _GetAvailableModules(const ValueList& args, SharedValue value);
		void _GetAvailableRuntimes(const ValueList& args, SharedValue value);
		void _GetBundledComponents(const ValueList& args, SharedValue value);
		void _GetBundledModules(const ValueList& args, SharedValue value);
		void _GetBundledRuntimes(const ValueList& args, SharedValue value);
	};
}

#endif
