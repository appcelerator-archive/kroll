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

#if defined(OS_WIN32)
// PHP wreaks havoc on all kinds of cdecl/export/inline/god knows what macros, causing
// math functions to be exported into each object file. _INC_MATH is the math inclusion macro;
// defining it here seems to fix this issue for now, but there's probably a better way.
// Also, undef inline and va_copy so we make sure to get the win32 versions of those for Poco.
// This is why preprocessor magic == evil
# define _INC_MATH
# include <zend_config.w32.h>
# include <sapi/embed/php_embed.h>
# undef inline
# undef va_copy
#else
# include <sapi/embed/php_embed.h>
#endif

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
