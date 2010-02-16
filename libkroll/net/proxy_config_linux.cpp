/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
#include <libproxy/proxy.h>
using std::string;
using std::wstring;
using std::vector;
using Poco::URI;
using Poco::StringTokenizer;

namespace kroll
{
	static pxProxyFactory* GetProxyFactory()
	{
		// TODO: We should free this on exit.
		static pxProxyFactory* factory = NULL;
		if (!factory)
		{
			factory = px_proxy_factory_new();
		}
		return factory;
	}

	namespace ProxyConfig
	{
		SharedPtr<Proxy> GetProxyForURLImpl(Poco::URI& uri)
		{
			std::string url(uri.toString());
			char* urlC = strdup(url.c_str());
			char** proxies = px_proxy_factory_get_proxies(GetProxyFactory(), urlC);
			free(urlC);

			// TODO(mrobinson): Instead of just returning the first applicable proxy, this
			// should return a list of them.
			const char* proxyChars = proxies[0];
			if (!proxyChars)
				return 0;

			// Do not pass in an entryScheme here (third argument), because it will
			// override the host scheme, which is the most important in this case.
			SharedProxy proxy(ProxyConfig::ParseProxyEntry(
				proxyChars, uri.getScheme(), string()));

			for (int i = 0; proxies[i]; i++)
				free(proxies[i]);
			free(proxies);

			return proxy;
		}
	}
}
