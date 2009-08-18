/**
 * Appcelerator Kroll - licensed under the Apache Public License 2  
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
#ifdef OS_WIN32
#include "proxy_config_win32.h"
#elif defined(OS_LINUX)
#include "proxy_config_linux.h"
#endif

using std::string;
using std::vector;
using Poco::Net::IPAddress;
using Poco::URI;

namespace kroll
{
	namespace
	{
		inline bool EndsWith(string haystack, string needle)
		{
			return haystack.find(needle) == (haystack.size() - needle.size());
		}
	}

	bool Proxy::IsIPAddress(string& str)
	{
		IPAddress address;
		return IPAddress::tryParse(str, address);
	}

	bool Proxy::ShouldBypass(URI& uri)
	{
		const std::string& uriHost = uri.getHost();
		const std::string& uriScheme = uri.getScheme();
		unsigned short uriPort = uri.getPort();
		for (size_t i = 0; i < bypassList.size(); i++)
		{
			SharedURI entry = bypassList.at(i);

			// An empty bypass entry equals an unconditional bypass.
			if (entry.isNull())
			{
				return true;
			}
			else
			{
				const std::string& entryHost = entry->getHost();
				const std::string& entryScheme = entry->getScheme();
				unsigned short entryPort = entry->getPort();

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
		}

		return false;
	}

	SharedURI Proxy::ProxyEntryToURI(string& entry)
	{
		SharedURI uri = new URI();

		size_t endScheme = entry.find("://");
		if (endScheme != string::npos)
		{
			uri->setScheme(entry.substr(0, endScheme));
			entry = entry.substr(endScheme + 3);
		}

		size_t scan = entry.size() - 1;
		while (scan > 0 && isdigit(entry[scan]))
			scan--;

		if (entry[scan] == ':' && scan != entry.size() - 1)
		{
			string portString = entry.substr(scan + 1);
			uri->setPort(atoi(portString.c_str()));
			entry = entry.substr(0, scan);
		}
		else
		{
			// Poco may have adjusted the port when we set the scheme
			// so force an accepting port here.
			uri->setPort(0);
		}

		uri->setHost(entry);
		return uri;
	}

	SharedProxyConfig ProxyConfig::instance = 0;
	SharedProxy ProxyConfig::GetProxyForURL(string& url)
	{
		// TODO: Only convert this to a Poco::URI once, instead of doing
		// it here and then again in the impls.
		URI uri(url);

		// Don't try to detect proxy settings for URLs we know are local
		std::string scheme(uri.getScheme());
		if (scheme == "app" || scheme == "ti" || scheme == "file")
			return 0;

#ifdef OS_WIN32
		if (instance.isNull())
		{
			instance = new Win32ProxyConfig();
		}
		return instance->GetProxyForURLImpl(url);
#elif defined(OS_LINUX)
		if (instance.isNull())
		{
			instance = new LinuxProxyConfig();
		}
		return instance->GetProxyForURLImpl(url);
#else
		return 0;
#endif
	}



}
