/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../utils.h"
#include "../kashmir/uuid.h"
#include "../kashmir/devrandom.h"
#include <sstream>

namespace UTILS_NS
{
namespace DataUtils
{
	std::string GenerateUUID()
	{
		kashmir::uuid_t uuid;
		kashmir::system::DevRandom devrandom;
		std::ostringstream outStream;
		devrandom >> uuid;
		outStream << uuid;
		return outStream.str();
	}
}
}
