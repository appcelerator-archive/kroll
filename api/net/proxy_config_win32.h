/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PROXY_CONFIG_WIN32_H_
#define _KR_PROXY_CONFIG_WIN32_H_
namespace kroll
{
	class WinHTTPSession;
	class Win32ProxyConfig : public ProxyConfig
	{
		public:
		Win32ProxyConfig();

		protected:
		bool useProxyAutoConfig;
		std::wstring autoConfigURL;
		std::vector<SharedProxy> ieProxies;
		SharedPtr<WinHTTPSession> session;

		virtual SharedProxy GetProxyForURLImpl(std::string& url);
		bool GetAutoProxiesForURL(std::string& url, std::vector<SharedProxy>&);
		static void ParseBypassList(std::string& bypassList,
			 std::vector<SharedURI>& bypassVector);
		static void ParseProxyList(std::string proxyList, std::string bypassList,
			std::vector<SharedProxy>& ieProxyList);
		static std::string ErrorCodeToString(DWORD code);
	};
}
#endif
