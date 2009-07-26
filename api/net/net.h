/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_NET_H_
#define _KR_NET_H_
#include <Poco/StringTokenizer.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/URI.h>

namespace kroll
{
	class Proxy;
	class ProxyConfig;
	typedef SharedPtr<Proxy> SharedProxy;
	typedef SharedPtr<ProxyConfig> SharedProxyConfig;
	typedef SharedPtr<Poco::URI> SharedURI;
}

#include "proxy_config.h"
#ifdef OS_WIN32
#include "proxy_config_win32.h"
#endif

#endif

