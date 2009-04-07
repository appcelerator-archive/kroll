/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "poco/DigestEngine.h"
#include "poco/MD5Engine.h"
#include "utils.h"

using KPoco::DigestEngine;
using KPoco::MD5Engine;

namespace kroll
{
	std::string DataUtils::HexMD5(std::string)
	{
		MD5Engine engine;
		engine.update("");
		return DigestEngine::digestToHex(engine.digest());
	}

}
