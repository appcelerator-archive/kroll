/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_MODULE_H
#define _PHP_MODULE_H

#include <sapi/embed/php_embed.h>
#include <kroll/kroll.h>

namespace kroll
{
    class PhpModule : public Module, public ModuleProvider
    {
        KROLL_MODULE_CLASS(PhpModule)

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
        static PhpModule* Instance()
        {
            return instance_;
        }

    private:
        SharedKObject binding;
        static PhpModule *instance_;
        DISALLOW_EVIL_CONSTRUCTORS(PhpModule);
    };
}

#include "php_module_instance.h"

#endif
