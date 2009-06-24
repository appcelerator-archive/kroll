/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PLATFORM_UTILS_H_
#define _KR_PLATFORM_UTILS_H_
#include <string>
#include "poco/KTypes.h"

namespace UTILS_NS
{
	typedef KPoco::KUInt8 NodeId[6]; /// Ethernet address.

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

		/**
		 * Get the machine ID of this machine from the .PRODUCT_NAME file
		 * or create it if it doesn't exist.
		 */
		static std::string GetMachineId();

		/**
		 * Create a machine ID for this machine and write it to the .PRODUCT_NAME file
		 * in the user runtime home directory. The machine id is specific to a particular
		 * user on a particular OS install and does not uniquely identify a machine.
		 */
		static std::string CreateMachineId();
	};
}
#endif
