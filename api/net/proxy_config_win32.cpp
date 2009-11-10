/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
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
namespace ProxyConfig
{

static vector<SharedPtr<BypassEntry> > bypassList;
static void InitializeWin32ProxyConfig();
static bool GetAutoProxiesForURL(string& url, vector<SharedProxy>& proxies,
	const string& scheme);
static string ErrorCodeToString(DWORD code);
static void AddToBypassList(string bypassListString,
	vector<SharedPtr<BypassEntry > >& list);

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
			WINHTTP_ACCESS_TYPE_NO_PROXY,
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

static bool useProxyAutoConfig = false;
static wstring autoConfigURL(L"");
static WinHTTPSession httpSession;
static vector<SharedProxy> ieProxies;

static void InitializeWin32ProxyConfig()
{
	static bool initialized = false;
	if (initialized)
		return;

	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
	ZeroMemory(&ieProxyConfig, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG)); 
	
	if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig))
	{
		if (ieProxyConfig.fAutoDetect)
		{
			GetLogger()->Debug("Proxy auto config activated by proxy settings");
			useProxyAutoConfig = true;
		}

		if (ieProxyConfig.lpszAutoConfigUrl != NULL)
		{
			// We using an auto proxy configuration, but this one
			// has a URL which we must contact to get the configuration info.
			autoConfigURL = ieProxyConfig.lpszAutoConfigUrl;
			GetLogger()->Debug("PAC URL: %s", WideToUTF8(autoConfigURL).c_str());
		}

		// We always keep IE proxy information in case auto proxy
		// determination fails, so we can use it as a fallback.
		if (ieProxyConfig.lpszProxy)
		{
			wstring proxyListW(ieProxyConfig.lpszProxy);
			string proxyList(WideToUTF8(proxyListW));

			GetLogger()->Debug("Parsing IE proxy list: '%s'", proxyList.c_str());
			ParseProxyList(proxyList, ieProxies);
		}

		if (ieProxyConfig.lpszProxyBypass)
		{
			wstring bypassW(ieProxyConfig.lpszProxyBypass);
			string bypassListString(WideToUTF8(bypassW));
			AddToBypassList(bypassListString, bypassList);
		}
	}
	else
	{
		// If there is no IE configuration information, we default to
		// attempting to get auto proxy information.
		useProxyAutoConfig = true;
	}

	if (useProxyAutoConfig || !autoConfigURL.empty())
	{
		// We failed to open an HINTERNET handle! WTF. We'll have to have
		// disable auto proxy support, because we can't do a lookup.
		if (!httpSession.GetHandle())
		{
			useProxyAutoConfig = false;
			autoConfigURL = L"";
		}
	}

	if (ieProxyConfig.lpszProxy)
		GlobalFree(ieProxyConfig.lpszProxy);
	if (ieProxyConfig.lpszProxyBypass)
		GlobalFree(ieProxyConfig.lpszProxyBypass);
	if (ieProxyConfig.lpszAutoConfigUrl)
		GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
}

// This method will return true if we should keep attempting to use a proxy
// or false if the auto proxy determination was to use a direct connection.
static bool GetAutoProxiesForURL(string& url, vector<SharedProxy>& proxies,
	const string& scheme)
{
	bool shouldUseProxy = true;
	WINHTTP_PROXY_INFO autoProxyInfo;
	ZeroMemory(&autoProxyInfo, sizeof(WINHTTP_PROXY_INFO)); 
	
	WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
	ZeroMemory(&autoProxyOptions, sizeof(WINHTTP_AUTOPROXY_OPTIONS)); 
	
	// This type of auto-detection might take several seconds, so
	// if the user specified an auto-configuration URL don't do it.
	// TODO: Maybe we should use this as a fallback later, but it's
	// *very* expensive. A better solution would be to use asyncronous
	// proxy determination.
	if (autoConfigURL.empty() && useProxyAutoConfig)
	{
		autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
		autoProxyOptions.dwAutoDetectFlags = 
			WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
	}

	if (!autoConfigURL.empty())
	{
		autoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
		autoProxyOptions.lpszAutoConfigUrl = autoConfigURL.c_str();
	}

	// From Chromium:
	// Per http://msdn.microsoft.com/en-us/library/aa383153(VS.85).aspx, it is
	// necessary to first try resolving with fAutoLogonIfChallenged set to false.
	// Otherwise, we fail over to trying it with a value of true.  This way we
	// get good performance in the case where WinHTTP uses an out-of-process
	// resolver.  This is important for Vista and Win2k3.
	wstring wideURL = UTF8ToWide(url);
	autoProxyOptions.fAutoLogonIfChallenged = FALSE;
	BOOL ok = WinHttpGetProxyForUrl(
		httpSession.GetHandle(), wideURL.c_str(), &autoProxyOptions, &autoProxyInfo);

	if (!ok && ERROR_WINHTTP_LOGIN_FAILURE == GetLastError())
	{
		autoProxyOptions.fAutoLogonIfChallenged = TRUE;
		ok = WinHttpGetProxyForUrl(
			httpSession.GetHandle(), wideURL.c_str(), &autoProxyOptions, &autoProxyInfo);
	}
	
	if (ok && autoProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY &&
		autoProxyInfo.lpszProxy)
	{
		if (autoProxyInfo.lpszProxyBypass)
		{
			wstring bypassW(autoProxyInfo.lpszProxyBypass);
			string bypassListString(WideToUTF8(bypassW));
			AddToBypassList(bypassListString, bypassList);
		}

		wstring proxyListW(autoProxyInfo.lpszProxy);
		string proxyList(WideToUTF8(proxyListW));

		GetLogger()->Debug("Parsing PAC result proxy list: '%s'", proxyList.c_str());
		ParseProxyList(proxyList, proxies, scheme);
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
		string error = "Could not get proxy for url=";
		error.append(url);
		error.append(": ");
		error.append(ErrorCodeToString(GetLastError()));
		Logger::Get("Proxy")->Error(error);
	}

	// Always cleanup
	if (autoProxyInfo.lpszProxy)
		GlobalFree(autoProxyInfo.lpszProxy);
	if (autoProxyInfo.lpszProxyBypass)
		GlobalFree(autoProxyInfo.lpszProxyBypass);

	return shouldUseProxy;
}

static void AddToBypassList(string bypassListString,
	vector<SharedPtr<BypassEntry > >& list)
{
	string sep = ",";
	StringTokenizer tokenizer(bypassListString, sep,
		StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
	for (size_t i = 0; i < tokenizer.count(); i++)
	{
		list.push_back(ParseBypassEntry(tokenizer[i]));
	}
}

static string ErrorCodeToString(DWORD code)
{
	// Okay. This is a little bit compulsive, but we really, really
	// want to get good debugging information for proxy lookup failures.
	if (code == ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR)
		return "ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR";
	else if (code == ERROR_WINHTTP_AUTODETECTION_FAILED)
		return "ERROR_WINHTTP_AUTODETECTION_FAILED";
	else if (code == ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT)
		return "ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT";
	else if (code == ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN)
		return "ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN";
	else if (code == ERROR_WINHTTP_CANNOT_CALL_AFTER_SEND)
		return "ERROR_WINHTTP_CANNOT_CALL_AFTER_SEND";
	else if (code == ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN)
		return "ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN";
	else if (code == ERROR_WINHTTP_CANNOT_CALL_BEFORE_SEND)
		return "ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN";
	else if (code == ERROR_WINHTTP_CANNOT_CONNECT)
		return "ERROR_WINHTTP_CANNOT_CONNECT";
	else if (code == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED)
		return "ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED";
	else if (code == ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW)
		return "ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW";
	else if (code == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED)
		return "ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED";
	else if (code == ERROR_WINHTTP_CONNECTION_ERROR)
		return "ERROR_WINHTTP_CONNECTION_ERROR";
	else if (code == ERROR_WINHTTP_HEADER_ALREADY_EXISTS)
		return "ERROR_WINHTTP_HEADER_ALREADY_EXISTS";
	else if (code == ERROR_WINHTTP_HEADER_COUNT_EXCEEDED)
		return "ERROR_WINHTTP_HEADER_COUNT_EXCEEDED";
	else if (code == ERROR_WINHTTP_HEADER_NOT_FOUND)
		return "ERROR_WINHTTP_HEADER_NOT_FOUND";
	else if (code == ERROR_WINHTTP_HEADER_SIZE_OVERFLOW)
		return "ERROR_WINHTTP_HEADER_SIZE_OVERFLOW";
	else if (code == ERROR_WINHTTP_INCORRECT_HANDLE_STATE)
		return "ERROR_WINHTTP_INCORRECT_HANDLE_STATE";
	else if (code == ERROR_WINHTTP_INCORRECT_HANDLE_TYPE)
		return "ERROR_WINHTTP_INCORRECT_HANDLE_TYPE";
	else if (code == ERROR_WINHTTP_INTERNAL_ERROR)
		return "ERROR_WINHTTP_INTERNAL_ERROR";
	else if (code == ERROR_WINHTTP_INVALID_OPTION)
		return "ERROR_WINHTTP_INVALID_OPTION";
	else if (code == ERROR_WINHTTP_INVALID_QUERY_REQUEST)
		return "ERROR_WINHTTP_INVALID_QUERY_REQUEST";
	else if (code == ERROR_WINHTTP_INVALID_SERVER_RESPONSE)
		return "ERROR_WINHTTP_INVALID_SERVER_RESPONSE";
	else if (ERROR_WINHTTP_INVALID_URL)
		return "ERROR_WINHTTP_INVALID_URL";
	else if (ERROR_WINHTTP_LOGIN_FAILURE)
		return "ERROR_WINHTTP_LOGIN_FAILURE";
	else if (ERROR_WINHTTP_NAME_NOT_RESOLVED)
		return "ERROR_WINHTTP_NAME_NOT_RESOLVED";
	else if (ERROR_WINHTTP_NOT_INITIALIZED)
		return "ERROR_WINHTTP_NOT_INITIALIZED";
	else if (ERROR_WINHTTP_OPERATION_CANCELLED)
		return "ERROR_WINHTTP_OPERATION_CANCELLED";
	else if (ERROR_WINHTTP_OPTION_NOT_SETTABLE)
		return "ERROR_WINHTTP_OPTION_NOT_SETTABLE";
	else if (ERROR_WINHTTP_OUT_OF_HANDLES)
		return "ERROR_WINHTTP_OUT_OF_HANDLES";
	else if (ERROR_WINHTTP_REDIRECT_FAILED)
		return "ERROR_WINHTTP_REDIRECT_FAILED";
	else if (ERROR_WINHTTP_RESEND_REQUEST)
		return "ERROR_WINHTTP_RESEND_REQUEST";
	else if (ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW)
		return "ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW";
	else if (ERROR_WINHTTP_SECURE_CERT_CN_INVALID)
		return "ERROR_WINHTTP_SECURE_CERT_CN_INVALID";
	else if (ERROR_WINHTTP_SECURE_CERT_DATE_INVALID)
		return "ERROR_WINHTTP_SECURE_CERT_DATE_INVALID";
	else if (ERROR_WINHTTP_SECURE_CERT_REV_FAILED)
		return "ERROR_WINHTTP_SECURE_CERT_REV_FAILED";
	else if (ERROR_WINHTTP_SECURE_CERT_REVOKED)
		return "ERROR_WINHTTP_SECURE_CERT_REVOKED";
	else if (ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE)
		return "ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE";
	else if (ERROR_WINHTTP_SECURE_CHANNEL_ERROR)
		return "ERROR_WINHTTP_SECURE_CHANNEL_ERROR";
	else if (ERROR_WINHTTP_SECURE_FAILURE)
		return "ERROR_WINHTTP_SECURE_FAILURE";
	else if (ERROR_WINHTTP_SECURE_INVALID_CA)
		return "ERROR_WINHTTP_SECURE_INVALID_CA";
	else if (ERROR_WINHTTP_SECURE_INVALID_CERT)
		return "ERROR_WINHTTP_SECURE_INVALID_CERT";
	else if (ERROR_WINHTTP_SHUTDOWN)
		return "ERROR_WINHTTP_SHUTDOWN";
	else if (ERROR_WINHTTP_TIMEOUT)
		return "ERROR_WINHTTP_TIMEOUT";
	else if (ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT)
		return "ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT";
	else if (ERROR_WINHTTP_UNRECOGNIZED_SCHEME)
		return "ERROR_WINHTTP_UNRECOGNIZED_SCHEME";
	else if (ERROR_NOT_ENOUGH_MEMORY)
		return "ERROR_NOT_ENOUGH_MEMORY";
	else if (ERROR_INSUFFICIENT_BUFFER)
		return "ERROR_INSUFFICIENT_BUFFER";
	else if (ERROR_INVALID_HANDLE)
		return "ERROR_INVALID_HANDLE";
	else if (ERROR_NO_MORE_FILES)
		return "ERROR_NO_MORE_FILES";
	else if (ERROR_NO_MORE_ITEMS)
		return "ERROR_NO_MORE_ITEMS";
	else
		return "UNKNOWN ERROR";
}

SharedProxy GetProxyForURLImpl(URI& uri)
{
	InitializeWin32ProxyConfig();
	string url(uri.toString());

	// If this URI is on the bypass list, just signal a direct connetion.
	if (ShouldBypass(uri, bypassList))
		return 0;

	// The auto proxy configuration might tell us to simply use
	// a direct connection, which should cause us to just return
	// null. Otherwise we should try to use the IE proxy list (next block)
	if (useProxyAutoConfig || !autoConfigURL.empty())
	{
		vector<SharedProxy> autoProxies;
		bool shouldUseIEProxy = GetAutoProxiesForURL(
			url, autoProxies, uri.getScheme());

		// TODO(mrobinson): This should eventually return a list.
		if (!autoProxies.empty())
			return autoProxies[0];

		if (!shouldUseIEProxy)
			return 0;
	}

	// Try the IE proxy list. Give preference to proxies set for a
	// particular scheme. If no proxy for that scheme can be found
	// use the SOCKS proxy as a fallback, if one exists.
	SharedProxy socksProxy(0);
	for (int i = 0; i < ieProxies.size(); i++)
	{
		SharedProxy proxy(ieProxies.at(i));
		ProxyType type = Proxy::SchemeToProxyType(uri.getScheme());
		if (type == proxy->type)
			return proxy;

		if (proxy->type == SOCKS && !socksProxy.get())
			socksProxy = proxy;
	}

	return socksProxy;
}

} // namespace ProxyConfig
} // namespace kroll

