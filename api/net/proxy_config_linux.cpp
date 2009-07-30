/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
#include "proxy_config_linux.h"
using std::string;
using std::wstring;
using std::vector;
using Poco::URI;
using Poco::StringTokenizer;

namespace kroll
{
	LinuxProxyConfig::LinuxProxyConfig() :
		proxyFactory(0)
	{
		proxyFactory = px_proxy_factory_new();
	}

	LinuxProxyConfig::~LinuxProxyConfig()
	{
		if (proxyFactory)
			px_proxy_factory_free(proxyFactory);	
	}

	SharedPtr<Proxy> LinuxProxyConfig::GetProxyForURLImpl(string& url)
	{
		char* urlCString = strdup(url.c_str());
		URI uri = URI(url);

		char** proxies = px_proxy_factory_get_proxies(
			proxyFactory, urlCString);
		free(urlCString);
		SharedPtr<Proxy> proxy = 0;

		// At this point we should really be trying each proxy until
		// we succeeed in being able to fetch this url. That doesn't
		// really jive with our way of doing things, so we'll try to
		// pick a likely candidate.
		int numberOfProxies = 0;
		for (int i = 0; proxies[i]; i++)
		{
			numberOfProxies++;
			SharedURI proxyURI = new URI(proxies[i]);
			if (proxyURI->getScheme() == "direct")
			{
				FreeProxies(proxies);
				return 0;
			}
			else if (proxyURI->getScheme() == uri.getScheme())
			{
				proxy = new Proxy;	
				proxy->info = proxyURI;
			}
		}

		// We didn't find a likely proxy, so just return the first one
		if (proxy.isNull() && numberOfProxies > 0)
		{
			proxy = new Proxy;	
			proxy->info = new URI(proxies[0]);
		}

		FreeProxies(proxies);
		return proxy;
	}

	void LinuxProxyConfig::FreeProxies(char** proxies)
	{
		for (int i = 0; proxies[i]; i++)
			free(proxies[i]);
		free(proxies);	

	}
}
