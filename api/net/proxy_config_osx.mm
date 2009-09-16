/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
using std::string;
using std::wstring;
using std::vector;
using Poco::URI;
using Poco::StringTokenizer;

namespace kroll
{
	namespace ProxyConfig
	{
		SharedPtr<Proxy> GetProxyForURLImpl(Poco::URI& uri)
		{
			return 0;
		}
	}
}
