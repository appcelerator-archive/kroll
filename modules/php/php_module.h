/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_MODULE_H_
#define _PHP_MODULE_H_

#include <string>
#include <vector>
#include <iostream>
#include <sapi/embed/php_embed.h>
#include <kroll/kroll.h>

#include "php_api.h"
#include "php_utils.h"
#include "k_php_list.h"
#include "php_evaluator.h"

namespace kroll
{
	class PHPModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(PHPModule)

		public:
			virtual bool IsModule(std::string& path);
			virtual Module* CreateModule(std::string& path);
			void InitializeBinding();

			virtual const char * GetDescription()
			{
				return "PHP Module Loader";
			}
			Host* GetHost()
			{
				return host;
			}
			static PHPModule* Instance()
			{
				return instance_;
			}

		private:
			SharedKObject binding;
			static PHPModule *instance_;
			DISALLOW_EVIL_CONSTRUCTORS(PHPModule);
		};
	}

#include "php_module_instance.h"

#endif
