/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PROXY_CONFIG_LINUX_H_
#define _KR_PROXY_CONFIG__H_
#include <libproxy/proxy.h>
namespace kroll
{
	class LinuxProxyConfig : public ProxyConfig
	{
		public:
		LinuxProxyConfig();
		virtual ~LinuxProxyConfig();

		protected:
		pxProxyFactory* proxyFactory;
		virtual SharedProxy GetProxyForURLImpl(std::string& url);
		void FreeProxies(char** proxies);
	};
}
#endif
