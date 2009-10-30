/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PROXY_CONFIG_H_
#define _KR_PROXY_CONFIG_H_
namespace kroll
{
	class KROLL_API BypassEntry
	{
	public:
		BypassEntry() : port(0) {}
		std::string scheme;
		std::string host;
		unsigned short port;
	};

	class KROLL_API Proxy
	{
	public:
		SharedURI info;
	};

	namespace ProxyConfig
	{
		KROLL_API void SetHTTPProxyOverride(SharedProxy);
		KROLL_API void SetHTTPSProxyOverride(SharedProxy);
		KROLL_API SharedProxy GetHTTPProxyOverride();
		KROLL_API SharedProxy GetHTTPSProxyOverride();
		KROLL_API SharedProxy GetProxyForURL(std::string& url);

		SharedProxy GetProxyForURLImpl(Poco::URI& uri);
		bool ShouldBypass(Poco::URI& uri,
			std::vector<SharedPtr<BypassEntry> >& bypassList);
		SharedPtr<BypassEntry> ParseBypassEntry(std::string entry);
		Logger* GetLogger();
	};
}
#endif
