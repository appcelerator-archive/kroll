/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"
#include "poco/KDigestEngine.h"
#include "poco/KMD5Engine.h"

using KPoco::DigestEngine;
using KPoco::MD5Engine;

namespace UTILS_NS
{
namespace DataUtils
{
	std::string HexMD5(std::string data)
	{
		MD5Engine engine;
		engine.update(data);
		return DigestEngine::digestToHex(engine.digest());
	}
}
}
