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

	class KROLL_API ProxyConfig
	{
		public:
		static SharedProxy GetProxyForURL(std::string& url);
		virtual SharedProxy GetProxyForURLImpl(std::string& url) = 0;
		virtual ~ProxyConfig(){}
	
		private:
		static SharedProxyConfig instance;
	};
}
#endif
