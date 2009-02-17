/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef PYTHONAPI_H_
#define PYTHONAPI_H_

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define KROLL_PYTHON_API EXPORT
#elif defined(OS_WIN32)
# ifdef KROLL_PYTHON_API_EXPORT
#  define KROLL_PYTHON_API __declspec(dllexport)
# else
#  define KROLL_PYTHON_API __declspec(dllimport)
# endif
#endif

#endif /* PYTHONAPI_H_ */
