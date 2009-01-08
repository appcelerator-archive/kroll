/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BASE_H_
#define _KR_BASE_H_

#ifdef __cplusplus
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#endif

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define KROLL_API EXPORT
#elif defined(OS_WIN32)
# ifdef KROLL_API_EXPORT
#  define KROLL_API __declspec(dllexport)
# else
#  define KROLL_API __declspec(dllimport)
# endif
# define EXPORT __declspec(dllexport)
#endif

#define VAL(str) #str
#define STRING(str) VAL(str)

#ifndef _PRODUCT_NAME
   #define _PRODUCT_NAME Kroll
#endif

#define PRODUCT_NAME STRING(_PRODUCT_NAME)

#ifndef _INSTALL_PREFIX
  #define _INSTALL_PREFIX "/usr/local"
#endif

#ifndef INSTALL_PREFIX
  #define INSTALL_PREFIX STRING(_INSTALL_PREFIX)
#endif


#ifdef DEBUG_REFCOUNT
	#define KR_ADDREF_RETURNING(t,o) (o!=NULL) ? (t)o->AddReference(__FILE__,__LINE__) : NULL
	#define KR_ADDREF(o) \
	if (o!=NULL) \
	{ \
		if (o->ReferenceCount()<1) fprintf(stderr,"!!!! Invalid Object. Reference Count < 1: %s:%d\n",__FILE__,__LINE__); \
		o->AddReference(__FILE__,__LINE__); \
	} 
	#define KR_DECREF(o) \
	if (o!=NULL) \
	{ \
		o->ReleaseReference(__FILE__,__LINE__); \
		o = NULL; \
	}
#else
	#define KR_ADDREF_RETURNING(t,o) (o!=NULL) ? (t)o->AddReference() : NULL
	#define KR_ADDREF(o) \
	if (o!=NULL) \
	{ \
		if (o->ReferenceCount()<1) fprintf(stderr,"!!!! Invalid Object. Reference Count < 1: %s:%d\n",__FILE__,__LINE__); \
		o->AddReference(); \
	} 
	#define KR_DECREF(o) \
	if (o!=NULL) \
	{ \
		o->ReleaseReference(); \
		o = NULL; \
	}
#endif

	
#define KR_UNUSED(o) if (o) {} 


#endif
