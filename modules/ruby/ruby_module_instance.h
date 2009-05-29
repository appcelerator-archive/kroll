/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_MODULE_INSTANCE_H
#define _RUBY_MODULE_INSTANCE_H

#include "ruby_module.h"

namespace kroll
{
	class RubyModuleInstance : public Module
	{
	public:
		RubyModuleInstance(Host *host, std::string path, std::string dir, std::string name);
	protected:
		virtual ~RubyModuleInstance();
	public:
		void Initialize ();
		void Destroy ();
	private:
		std::string path;
		DISALLOW_EVIL_CONSTRUCTORS(RubyModuleInstance);
	};
}

#endif
