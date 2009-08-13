/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PHPAPI_H_
#define _PHPAPI_H_

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define KROLL_PHP_API EXPORT
#elif defined(OS_WIN32)
# ifdef KROLL_PHP_API_EXPORT
#  define KROLL_PHP_API __declspec(dllexport)
# else
#  define KROLL_PHP_API __declspec(dllimport)
# endif
#endif

#endif /* PHPAPI_H_ */

