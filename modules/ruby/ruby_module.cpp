/** Appcelerator Kroll - licensed under the Apache Public License 2 see LICENSE
 * in the root folder for details on the license.  Copyright (c) 2008
 * Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "ruby_module.h"
#include <Poco/Path.h>

extern "C" EXPORT RubyModule* CreateModule(Host *host, const char* path)
{
	return new RubyModule(host, path);
}

static const char* supportedScriptTypes[3] = {"rb", "ruby", NULL};

namespace kroll
{
	RubyModule* RubyModule::instance_ = NULL;

	void RubyModule::Initialize()
	{
		RubyModule::instance_ = this;

		ruby_init();
		ruby_init_loadpath();

		// Add the application directoy to the Ruby include path so
		// that includes work in a intuitive way for application developers.
		ruby_incpush(UTF8ToSystem(host->GetApplication()->GetResourcesPath()).c_str());

        host->script()->AddInterpreter(&interpreter, supportedScriptTypes);

		host->AddModuleProvider(this);
	}

	void RubyModule::Stop()
	{
        host->script()->RemoveInterpreter(&interpreter);

		RubyModule::instance_ = NULL;
		ruby_cleanup(0);
	}

	const static std::string ruby_suffix = "module.rb";
	bool RubyModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-ruby_suffix.length()) == ruby_suffix);
	}

	Module* RubyModule::CreateModule(std::string& path)
	{
		path = UTF8ToSystem(path);
		rb_load_file(path.c_str());
		ruby_exec();
		// TODO: Do we need to call ruby_cleanup() here?

		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-ruby_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		return new RubyModuleInstance(host, path, moduledir, name);
	}

}
