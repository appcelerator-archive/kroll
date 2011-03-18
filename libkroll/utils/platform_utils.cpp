/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "utils.h"

#include <stdio.h>
#include <string.h>

namespace UTILS_NS
{
namespace PlatformUtils
{
	std::string GetFirstMACAddress()
	{
		MACAddress address;
		memset(&address, 0, sizeof(MACAddress));

		try
		{
			PlatformUtils::GetFirstMACAddressImpl(address);
		}
		catch(...)
		{
			// Just return zeros.
		}

		char result[18];
		std::sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x",
			address[0], address[1], address[2], address[3],
			address[4], address[5]);
		return std::string(result);
	}

	std::string GetMachineId()
	{
        static std::string machineID;
        if (machineID.empty())
            machineID = DataUtils::HexMD5(GetFirstMACAddress());
        return machineID;
	}
}
}
