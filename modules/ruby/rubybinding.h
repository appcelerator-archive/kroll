/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef RUBYBINDING_H_
#define RUBYBINDING_H_

#include <api/binding/bound_object.h>
#include <api/module.h>
#include <ruby.h>

namespace kroll
{
	class RubyBinding: public BoundObject {
	protected:
		VALUE ruby_object;
	public:
		RubyBinding(VALUE ruby_object);
		virtual ~RubyBinding() { }

		void RubyInvoke(const char *name, const ArgList& args, ReturnValue *returnValue);
		void RubyGet(const char *name, ReturnValue *returnValue);
		void RubySet(const char *name, const ArgValue& value);

	};

	class RubyModuleInstance : public Module {
	protected:
		std::string path;
	public:
		RubyModuleInstance(Host *host, std::string path_)
			: path(path_),
			Module(host) {}

		const char* GetName() { return path.c_str(); }
		void Initialize ();
		void Destroy ();
	};
}

#endif /* RUBYBINDING_H_ */
