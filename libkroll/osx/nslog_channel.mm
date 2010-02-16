/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{
	NSLogChannel::NSLogChannel()
		: formatter("[%s] [%p] %t")
	{
	}

	void NSLogChannel::log(const Poco::Message& msg)
	{
		std::string text;
		formatter.format(msg, text);
		NSLog(@"%s", text.c_str());
	}
}
