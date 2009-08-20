/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_TYPES_H_
#define _PHP_TYPES_H_

#include <typeinfo>
#include "php_module.h"

namespace kroll
{
	class PHPUtils
	{
		public:
		static SharedValue ToKrollValue(zval* value);
		static zval* ToPHPValue(SharedValue value);
		static void ToPHPValue(SharedValue value, zval** returnValue);
		static std::string ZValToPropertyName(zval* property);
		static void CreatePHPKObject(SharedValue objectValue, zval** returnValue);

		private:
		PHPUtils() {}
		~PHPUtils() {}
	};
}

#endif
