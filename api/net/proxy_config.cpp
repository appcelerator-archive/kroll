/**
 * Appcelerator Kroll - licensed under the Apache Public License 2 * see LICENSE in the root folder for details on the license.
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
	namespace
	{
		inline bool EndsWith(string haystack, string needle)
		{
			return std::mismatch(
				needle.rbegin(), needle.rend(),
				haystack.rbegin()).first != needle.rend();
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
		
		std::string::const_iterator begin = entry.begin();
		std::string::const_iterator scan = entry.end() - 1;
		while (scan > begin && isdigit(*scan))
			scan--;

		if (*scan == ':' && scan != entry.end())
		{
				string portString = string(scan + 1, entry.end());
				uri->setPort(atoi(portString.c_str()));
				entry = string(begin, scan);	
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
#ifdef OS_WIN32
		if (instance.isNull())
		{
			instance = new Win32ProxyConfig();
		}
		return instance->GetProxyForURLImpl(url);
#else
		return 0;
#endif
	}



}
