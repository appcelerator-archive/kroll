/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PROXY_CONFIG_H_
#define _KR_PROXY_CONFIG_H_
namespace kroll
{
	class KROLL_API Proxy
	{
		public:
		SharedURI info;
		std::vector<SharedURI> bypassList;
		bool ShouldBypass(Poco::URI& uri);
		static bool IsIPAddress(std::string& str);
		static SharedURI ProxyEntryToURI(std::string& entry);
	};

	namespace ProxyConfig
	{
		KROLL_API SharedProxy GetProxyForURL(std::string& url);
		KROLL_API SharedProxy GetProxyForURLImpl(Poco::URI& uri);
	};
}
#endif
