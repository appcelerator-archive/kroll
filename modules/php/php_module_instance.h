/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_MODULE_INSTANCE_H_
#define _PHP_MODULE_INSTANCE_H_

#include "php_module.h"

namespace kroll
{
	class PHPModuleInstance : public Module
	{
		public:
		PHPModuleInstance(Host *host, std::string path, std::string dir, std::string name);
		void Initialize();
		void Destroy();

		protected:
		virtual ~PHPModuleInstance();

		private:
		std::string path;
		DISALLOW_EVIL_CONSTRUCTORS(PHPModuleInstance);
	};
}

#endif
