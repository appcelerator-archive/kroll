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
#include <sstream>

/* 
 * PHP wreaks havoc on all kinds of cdecl/export/inline/god knows what macros,
 * causing math functions to be exported into each object file. _INC_MATH is
 * the math inclusion macro; defining it here seems to fix this issue for now,
 * but there's probably a better way. Also, undef inline and va_copy so we make
 * sure to get the win32 versions of those for Poco. This is why preprocessor
 * magic == evil
 */
#if defined(OS_WIN32)
#define _INC_MATH
#include <zend_config.w32.h>
#include <sapi/embed/php_embed.h>
#undef inline
#undef va_copy
#undef PARSE_SERVER
#else
#include <sapi/embed/php_embed.h>
#endif

#include <kroll/kroll.h>
#include <sstream>
#include <stack>

#include "php_api.h"
#include "php_utils.h"
#include "k_php_object.h"
#include "k_php_method.h"
#include "k_php_function.h"
#include "k_php_list.h"
#include "k_php_array_object.h"
#include "php_evaluator.h"

#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_API.h>
#include <Zend/zend_closures.h>
#include <Zend/zend_hash.h>

namespace kroll
{
	class PHPModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(PHPModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		void InitializeBinding();

		virtual const char * GetDescription() { return "PHP Module Loader"; }
		static PHPModule* Instance() { return instance_; }
		static void SetBuffering(bool buffering);
		static std::ostringstream& GetBuffer() { return buffer; }
		static std::string& GetMimeType() { return mimeType; }
		
		void PushURI(Poco::URI& uri) { uriStack.push(new Poco::URI(uri)); }
		void PopURI() { Poco::URI* uri = uriStack.top(); uriStack.pop(); delete uri; }
		Poco::URI* GetURI() { return uriStack.size() == 0 ? 0 : uriStack.top(); }
		
	private:
		KObjectRef binding;
		static std::ostringstream buffer;
		static std::string mimeType;
		static PHPModule *instance_;
		std::stack<Poco::URI*> uriStack;

		DISALLOW_EVIL_CONSTRUCTORS(PHPModule);
	};
}

#include "php_module_instance.h"

#endif
