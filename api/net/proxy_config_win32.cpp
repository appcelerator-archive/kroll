/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
#include "proxy_config_win32.h"
#define _WINSOCKAPI_
#include <winsock2.h>
#include <windows.h>
#include <new.h>
#include <objbase.h>
#include <winhttp.h>
using std::string;
using std::wstring;
using std::vector;
using Poco::URI;
using Poco::StringTokenizer;

namespace kroll
{
	class WinHTTPSession
	{
		public:
		WinHTTPSession() :
			handle(NULL)
		{
			// Just use something reasonable as a user agent.
			this->handle = WinHttpOpen(
				L"Mozilla/5.0 (Windows; U; Windows NT 5.1; en) "
				L"AppleWebKit/526.9 (KHTML, like Gecko) "
				L"Version/4.0dp1 Safari/526.8",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME, 
				WINHTTP_NO_PROXY_BYPASS, 0);
		}

		inline HINTERNET GetHandle()
		{
			return handle;
		}

		~WinHTTPSession()
		{
			if (handle)
				WinHttpCloseHandle(handle);
		}

		private:
		HINTERNET handle;
	};

	Win32ProxyConfig::Win32ProxyConfig() :
		useAutoProxy(false),
		autoConfigURL(""),
		session(0)
	{
		WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
		ZeroMemory(&ieProxyConfig, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG)); 
		
		if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig))
		{
			if (ieProxyConfig.fAutoDetect)
			{
				this->useAutoProxy = true;
			}
	
			if (ieProxyConfig.lpszAutoConfigUrl != NULL)
			{
				// Not only are we using an auto proxy configuration, but this one
				// has a URL which we must contact to get the configuration info.
				this->useAutoProxy = true;

				std::wstring autoConfigURLW = ieProxyConfig.lpszAutoConfigUrl;
				this->autoConfigURL = string(autoConfigURLW.begin(), autoConfigURLW.end());
			}

			// We always keep IE proxy information in case auto proxy
			// determination fails. We want to it as a fallback.
			if (ieProxyConfig.lpszProxy)
			{
				std::string bypassList;
				if (ieProxyConfig.lpszProxyBypass)
				{
					std::wstring bypassW = ieProxyConfig.lpszProxyBypass;
					bypassList = string(bypassW.begin(), bypassW.end());
				}

				std::wstring proxyListW = ieProxyConfig.lpszProxy;
				string proxyList = string(proxyListW.begin(), proxyListW.end());
				ParseProxyList(proxyList, bypassList, ieProxies);
			}
		}
		else
		{
			// If there is no IE configuration information, we default to
			// attempting to get auto proxy information.
			useAutoProxy = true;
		}

		if (this->useAutoProxy)
		{
			session = new WinHTTPSession();

			// We failed to open an HINTERNET handle! WTF. We'll have to have
			// disable auto proxy support, because we can't do a lookup.
			if (!session->GetHandle())
				this->useAutoProxy = false;
		}

		if (ieProxyConfig.lpszProxy)
			GlobalFree(ieProxyConfig.lpszProxy);
		if (ieProxyConfig.lpszProxyBypass)
			GlobalFree(ieProxyConfig.lpszProxyBypass);
		if (ieProxyConfig.lpszAutoConfigUrl)
			GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
	}

	SharedPtr<Proxy> Win32ProxyConfig::GetProxyForURLImpl(string& url)
	{
		URI uri = URI(url);

		// The auto proxy configuration might tell us to simply use
		// a direct connection, which should cause us to just return
		// null. Otherwise we should try to use the IE proxy list (next block)
		if (useAutoProxy)
		{
			std::vector<SharedProxy> autoProxies;
			bool shouldUseIEProxy = GetAutoProxiesForURL(url, autoProxies);

			for (int i = 0; i < autoProxies.size(); i++)
			{
				SharedProxy proxy = autoProxies.at(i);
				if (proxy->ShouldBypass(uri))
				{
					return 0;
				}
				else if (proxy->info->getScheme() == uri.getScheme())
				{
					return proxy;
				}
			}

			if (!shouldUseIEProxy)
				return 0;
		}

		// Try the IE proxy list
		for (int i = 0; i < ieProxies.size(); i++)
		{
			SharedProxy proxy = ieProxies.at(i);
			std::string proxyScheme = proxy->info->getScheme();
			if (proxy->ShouldBypass(uri))
			{
				return 0;
			}
			else if (proxyScheme.empty() || proxyScheme == uri.getScheme())
			{
				return proxy;
			}
		}

		return 0;
	}


	// This method will return true if we should keep attempting to use a proxy
	// or false if the auto proxy determination was to use a direct connection.
	bool Win32ProxyConfig::GetAutoProxiesForURL(
		string& url, vector<SharedProxy>& proxies)
	{
		bool shouldUseProxy = true;
		WINHTTP_PROXY_INFO autoProxyInfo;
		ZeroMemory(&autoProxyInfo, sizeof(WINHTTP_PROXY_INFO)); 
		
		WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
		ZeroMemory(&autoProxyOptions, sizeof(WINHTTP_AUTOPROXY_OPTIONS)); 
		
		if (this->autoConfigURL.empty())
		{
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
			autoProxyOptions.dwAutoDetectFlags = 
				WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		}
		else
		{
			autoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;

			wstring autoConfigURLW = wstring(autoConfigURL.begin(), autoConfigURL.end());
			autoProxyOptions.lpszAutoConfigUrl = autoConfigURLW.c_str();
		}
		
		// From Chromium:
		// Per http://msdn.microsoft.com/en-us/library/aa383153(VS.85).aspx, it is
		// necessary to first try resolving with fAutoLogonIfChallenged set to false.
		// Otherwise, we fail over to trying it with a value of true.  This way we
		// get good performance in the case where WinHTTP uses an out-of-process
		// resolver.  This is important for Vista and Win2k3.
		wstring urlw = wstring(url.begin(), url.end());
		autoProxyOptions.fAutoLogonIfChallenged = FALSE;
		BOOL ok = WinHttpGetProxyForUrl(
			session->GetHandle(), urlw.c_str(), &autoProxyOptions, &autoProxyInfo);
		
		if (!ok && ERROR_WINHTTP_LOGIN_FAILURE == GetLastError())
		{
			autoProxyOptions.fAutoLogonIfChallenged = TRUE;
			ok = WinHttpGetProxyForUrl(
				session->GetHandle(), urlw.c_str(), &autoProxyOptions, &autoProxyInfo);
		}
		
		if (ok && autoProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY &&
			 autoProxyInfo.lpszProxy)
		{
			// Only the first proxy in the list will get a copy of the bypass list.
			std::string bypassList;
			if (autoProxyInfo.lpszProxyBypass)
			{
				std::wstring bypassW = autoProxyInfo.lpszProxyBypass;
				bypassList = string(bypassW.begin(), bypassW.end());
			}

			std::wstring proxyListW = autoProxyInfo.lpszProxy;
			string proxyList = string(proxyListW.begin(), proxyListW.end());
			ParseProxyList(proxyList, bypassList, proxies);
		}
		else if (ok && autoProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NO_PROXY)
		{
			// The only case in which we do not continue to try using a proxy.
			// In this case the auto proxy setup told us to use a direct connection.
			shouldUseProxy = false;
		}
		else
		{
			// Auto proxy failed, so try another method
			useAutoProxy = false;
			string error = "Could not get proxy for url=";
			error.append(url);
			error.append(": ");
			error.append(Win32Utils::QuickFormatMessage(GetLastError()));
			Logger::Get("Proxy")->Error(error);
		}
		
		// Always cleanup
		if (autoProxyInfo.lpszProxy)
			GlobalFree(autoProxyInfo.lpszProxy);
		if (autoProxyInfo.lpszProxyBypass)
			GlobalFree(autoProxyInfo.lpszProxyBypass);
		
		return shouldUseProxy;
	}

	void Win32ProxyConfig::ParseBypassList(string& bypassList,
		 vector<SharedURI>& bypassVector)
	{
		std::string sep = ",";
		StringTokenizer tokenizer(bypassList, sep,
			StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
		for (size_t i = 0; i < tokenizer.count(); i++)
		{
			// Traditionally an endswith comparison is always done with the host
			// part, so we throwaway explicit wildcards at the beginning. If the
			// entire string is a wildcard this is an unconditional bypass.
			std::string entry = tokenizer[i];
			if (entry.at(0) == '*' && entry.size() == 1)
			{
				// Null URI means always bypass
				bypassVector.push_back(0);
				continue;
			}
			else if (entry.at(0) == '*' && entry.size() > 1)
			{
				entry = entry.substr(1);
			}

			SharedPtr<URI> bypassEntry = Proxy::ProxyEntryToURI(entry);
			bypassVector.push_back(bypassEntry);
		}
	}

	void Win32ProxyConfig::ParseProxyList(
		string proxyList, string bypassList,
		vector<SharedPtr<Proxy > >& ieProxyList)
	{
		std::string sep = "; ";
		StringTokenizer proxyTokens(proxyList, sep,
			StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
		for (size_t i = 0; i < proxyTokens.count(); i++)
		{
			string entry = proxyTokens[i];
			string scheme = "";
			bool forScheme = false;

			size_t schemeEnd = entry.find('=');
			if (schemeEnd != string::npos)
			{
				// This proxy only applies to the scheme before '='
				scheme = entry.substr(0, schemeEnd - 1);
				entry = entry.substr(schemeEnd + 1);
			}

			SharedPtr<Proxy> proxy = new Proxy;	
			proxy->info = Proxy::ProxyEntryToURI(entry);

			// Set the scheme based on the value that came before '='
			// We want this proxy to only apply to what it was specifically
			// set for.
			proxy->info->setScheme(scheme);
			ieProxyList.push_back(proxy);
		}

		if (ieProxyList.size() > 0 && !bypassList.empty())
		{
			SharedPtr<Proxy> proxy = ieProxyList.at(0);
			ParseBypassList(bypassList, proxy->bypassList);
		}
	}
}
