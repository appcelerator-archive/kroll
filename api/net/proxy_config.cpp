/**
 * Appcelerator Kroll - licensed under the Apache Public License 2  
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"

using std::string;
using std::vector;
using Poco::Net::IPAddress;
using Poco::URI;

namespace kroll
{
namespace ProxyConfig
{
	SharedProxy httpProxyOverride(0);
	SharedProxy httpsProxyOverride(0);

	void SetHTTPProxyOverride(SharedProxy newProxyOverride)
	{
		httpProxyOverride = newProxyOverride;
	}

	SharedProxy GetHTTPProxyOverride()
	{
		return httpProxyOverride;
	}

	void SetHTTPSProxyOverride(SharedProxy newProxyOverride)
	{
		httpsProxyOverride = newProxyOverride;
	}

	SharedProxy GetHTTPSProxyOverride()
	{
		return httpsProxyOverride;
	}

	SharedProxy GetProxyForURL(string& url)
	{
		static Logger* logger = GetLogger();
		URI uri(url);

		// Don't try to detect proxy settings for URLs we know are local
		std::string scheme(uri.getScheme());
		if (scheme == "app" || scheme == "ti" || scheme == "file")
			return 0;

		if (scheme == "http" && !httpProxyOverride.isNull())
			return httpProxyOverride;

		if (scheme == "https" && !httpsProxyOverride.isNull())
			return httpsProxyOverride;

		logger->Debug("Looking up proxy information for: %s", url.c_str());
		SharedProxy proxy(ProxyConfig::GetProxyForURLImpl(uri));

		if (proxy.isNull())
			logger->Debug("Using direct connection.");
		else
			logger->Debug("Using proxy (scheme=%s host=%s port=%i)",
				proxy->info->getScheme().c_str(),
				proxy->info->getHost().c_str(), proxy->info->getPort());

		return proxy;
	}

	static inline bool EndsWith(string haystack, string needle)
	{
		return haystack.find(needle) == (haystack.size() - needle.size());
	}

	static bool ShouldBypassWithEntry(URI& uri, SharedPtr<BypassEntry> entry)
	{
		const std::string& uriHost = uri.getHost();
		const std::string& uriScheme = uri.getScheme();
		unsigned short uriPort = uri.getPort();
		const std::string& entryHost = entry->host;
		const std::string& entryScheme = entry->scheme;
		unsigned short entryPort = entry->port;

		GetLogger()->Debug("bypass entry: scheme='%s' host='%s' port='%i'", 
			entry->scheme.c_str(), entry->host.c_str(), entry->port);

		// An empty bypass entry equals an unconditional bypass.
		if (entry.isNull())
		{
			return true;
		}
		else
		{
			if (entryHost == "<local>" && uriHost.find(".") == string::npos)
			{
				return true;
			}
			else if (EndsWith(uriHost, entryHost) &&
				(entryScheme.empty() || entryScheme == uriScheme) &&
				(entryPort == 0 || entryPort == uriPort))
			{
				return true;
			}
		}

		return false;
	}

	bool ShouldBypass(URI& uri, vector<SharedPtr<BypassEntry> >& bypassList)
	{
		GetLogger()->Debug("Checking whether %s should be bypassed.", 
			uri.toString().c_str());

		for (size_t i = 0; i < bypassList.size(); i++)
		{
			if (ShouldBypassWithEntry(uri, bypassList.at(i)))
				return true;
		}

		GetLogger()->Debug("No bypass");
		return false;
	}

	SharedPtr<BypassEntry> ParseBypassEntry(string entry)
	{
		// Traditionally an endswith comparison is always done with the host
		// part, so we throw away explicit wildcards at the beginning. If the
		// entire string is a wildcard this is an unconditional bypass.
		if (entry.at(0) == '*' && entry.size() == 1)
		{
			// Null URI means always bypass
			return 0;
		}
		else if (entry.at(0) == '*' && entry.size() > 1)
		{
			entry = entry.substr(1);
		}

		SharedPtr<BypassEntry> bypass(new BypassEntry());
		size_t endScheme = entry.find("://");
		if (endScheme != string::npos)
		{
			bypass->scheme = entry.substr(0, endScheme);
			entry = entry.substr(endScheme + 3);
		}

		size_t scan = entry.size() - 1;
		while (scan > 0 && isdigit(entry[scan]))
			scan--;

		if (entry[scan] == ':' && scan != entry.size() - 1)
		{
			string portString = entry.substr(scan + 1);
			bypass->port = atoi(portString.c_str());
			entry = entry.substr(0, scan);
		}

		bypass->host = entry;
		return bypass;
	}

	Logger* GetLogger()
	{
		static Logger* logger = Logger::Get("Proxy");
		return logger;
	}

} // namespace ProxyConfig
} // namespace kroll
