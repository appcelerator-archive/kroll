/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PLATFORM_UTILS_H_
#define _KR_PLATFORM_UTILS_H_
#include <string>

namespace kroll
{

#if defined(OS_WIN32)
	//
	// Windows/Visual C++
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed __int64         Int64;
	typedef unsigned __int64       UInt64;
	#if defined(OS_64)
		#define POCO_PTR_IS_64_BIT 1
		typedef signed __int64     IntPtr;
		typedef unsigned __int64   UIntPtr;
	#else
		typedef signed long        IntPtr;
		typedef unsigned long      UIntPtr;
	#endif
	#define POCO_HAVE_INT64 1

#elif defined(OS_LINUX) || defined (OS_OSX)
	//
	// Unix/GCC
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;
	#if defined(OS_64)
		#define POCO_PTR_IS_64_BIT 1
		#define POCO_LONG_IS_64_BIT 1
		typedef signed long        Int64;
		typedef unsigned long      UInt64;
	#else
		typedef signed long long   Int64;
		typedef unsigned long long UInt64;
	#endif
	#define POCO_HAVE_INT64 1
#endif

	typedef UInt8 NodeId[6]; /// Ethernet address.

	class KROLL_API PlatformUtils
	{
		public:
		/*
		 * Get the first MAC address of this computer
		 * @returns the first MAC address in standard dotted format
		 */
		static std::string GetFirstMACAddress();

		/*
		 * Get the first MAC address of this computer
		 * @returns the first MAC address in standard dotted format
		 */
		static void GetNodeId(NodeId&);
	};
}
#endif
