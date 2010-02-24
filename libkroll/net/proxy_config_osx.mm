/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

/*
 * Thanks! -->
 * Portions of this code come from Chromium: src/net/proxy/proxy_resolver_mac.cc
 * and are covered by the Chromium license. Please see the CREDITS file in
 * the root directory of the project for more information.
 *
 * Portions of this code come are based on CFProxySupportTool at:
 * http://developer.apple.com/mac/library/samplecode/CFProxySupportTool/
 */

#include "../kroll.h"
#include "net.h"

#include <Poco/Thread.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>

using std::string;
using std::vector;
using Poco::URI;
using Poco::StringTokenizer;

namespace kroll
{
namespace ProxyConfig
{

static string autoConfigURL;
static SharedProxy httpProxy(0);
static SharedProxy httpsProxy(0);
static SharedProxy ftpProxy(0);
static SharedProxy socksProxy(0);
static vector<SharedPtr<BypassEntry> > bypassList;
static bool bypassLocalNames = false;

struct PACRequestInfo
{
	CFURLRef url;
	CFURLRef scriptURL;
	CFMutableArrayRef proxies;
};

// Utility function to pull out a value from a dictionary, check its type, and
// return it. Returns NULL if the key is not present or of the wrong type.
static CFTypeRef GetValueFromDictionary(CFDictionaryRef dict, CFStringRef key,
	CFTypeID expected_type)
{
	CFTypeRef value = CFDictionaryGetValue(dict, key);
	if (!value)
		return value;

	if (CFGetTypeID(value) != expected_type)
		return NULL;

	return value;
}

// Utility function to pull out a boolean value from a dictionary and return it,
// returning a default value if the key is not present.
static bool GetBoolFromDictionary(CFDictionaryRef dict, CFStringRef key,
	bool defaultValue)
{
	CFNumberRef number = (CFNumberRef)GetValueFromDictionary(dict, key,
		CFNumberGetTypeID());

	if (!number)
		return defaultValue;

	int intValue;
	if (CFNumberGetValue(number, kCFNumberIntType, &intValue))
		return intValue;
	else
		return defaultValue;
}

// Utility function to pull out a host/port pair from a dictionary and return it
// as a ProxyServer object. Pass in a dictionary that has a  value for the host
// key and optionally a value for the port key. In the error condition where
// the host value is especially malformed, returns an invalid ProxyServer.
SharedProxy GetProxyServerFromDictionary(string scheme,
	CFDictionaryRef dict, CFStringRef hostKey, CFStringRef portKey)
{
	CFStringRef hostRef = (CFStringRef) GetValueFromDictionary(
		dict, hostKey, CFStringGetTypeID());

	if (!hostRef)
	{
		GetLogger()->Warn("Could not find the host key in the proxy dictionary.");
		return 0;
	}

	// This should work equally well for hostnames without a port.
	string host(kroll::CFStringToUTF8(hostRef));
	SharedProxy proxy(ParseProxyEntry(host, scheme, scheme));

	CFNumberRef portRef = (CFNumberRef) GetValueFromDictionary(
		dict, portKey, CFNumberGetTypeID());

	if (portRef)
	{
		int port;
		CFNumberGetValue(portRef, kCFNumberIntType, &port);
		proxy->port = port;
	}

	GetLogger()->Debug("Got proxy from dictionary: %s", proxy->ToString().c_str());
	return proxy;
}

static void InitializeOSXProxyConfig()
{
	static bool initialized = false;
	if (initialized)
		return;

	CFRef<CFDictionaryRef> configDict(SCDynamicStoreCopyProxies(NULL));

	// PAC file
	if (GetBoolFromDictionary(configDict.get(),
		kSCPropNetProxiesProxyAutoConfigEnable, false))
	{
		CFStringRef pacURLRef = (CFStringRef) GetValueFromDictionary(
			configDict.get(), kSCPropNetProxiesProxyAutoConfigURLString,
			CFStringGetTypeID());

		if (pacURLRef)
		{
			autoConfigURL = kroll::CFStringToUTF8(pacURLRef);
			GetLogger()->Debug("Using PAC URL: %s", autoConfigURL.c_str());
		}
	}

	// FTP proxy
	if (GetBoolFromDictionary(configDict.get(), kSCPropNetProxiesFTPEnable, false))
	{
		ftpProxy = GetProxyServerFromDictionary("ftp", configDict.get(),
			kSCPropNetProxiesFTPProxy, kSCPropNetProxiesFTPPort);

		if (!ftpProxy.isNull())
			GetLogger()->Debug("FTP Proxy: %s", ftpProxy->ToString().c_str());
	}

	// HTTP proxy
	if (GetBoolFromDictionary(configDict.get(), kSCPropNetProxiesHTTPEnable, false))
	{
		httpProxy = GetProxyServerFromDictionary("http", configDict.get(),
			kSCPropNetProxiesHTTPProxy, kSCPropNetProxiesHTTPPort);

		if (!httpProxy.isNull())
			GetLogger()->Debug("HTTP Proxy: %s", httpProxy->ToString().c_str());
	}

	// HTTPS proxy
	if (GetBoolFromDictionary(configDict.get(), kSCPropNetProxiesHTTPSEnable, false))
	{
		httpsProxy = GetProxyServerFromDictionary("https", configDict.get(),
			kSCPropNetProxiesHTTPSProxy, kSCPropNetProxiesHTTPSPort);

		if (!httpsProxy.isNull())
			GetLogger()->Debug("HTTPS Proxy: %s", httpsProxy->ToString().c_str());
	}

	// SOCKS proxy
	if (GetBoolFromDictionary(configDict.get(), kSCPropNetProxiesSOCKSEnable, false))
	{
		socksProxy = GetProxyServerFromDictionary("socks", configDict.get(),
			kSCPropNetProxiesSOCKSProxy, kSCPropNetProxiesSOCKSPort);

		if (!socksProxy.isNull())
			GetLogger()->Debug("SOCKS Proxy: %s", socksProxy->ToString().c_str());
	}

	// Proxy bypass list
	CFArrayRef bypassArrayRef = (CFArrayRef) GetValueFromDictionary(configDict.get(),
		kSCPropNetProxiesExceptionsList, CFArrayGetTypeID());

	if (bypassArrayRef)
	{
		CFIndex bypassArrayCount = CFArrayGetCount(bypassArrayRef);
		for (CFIndex i = 0; i < bypassArrayCount; i++)
		{
			CFStringRef bypassItemRef = (CFStringRef )CFArrayGetValueAtIndex(
				bypassArrayRef, i);

			if (CFGetTypeID(bypassItemRef) != CFStringGetTypeID())
			{
				GetLogger()->Warn("Excepted string value for item in bypass list.");
			}
			else
			{
				string entry(kroll::CFStringToUTF8(bypassItemRef));
				GetLogger()->Debug("Proxy bypass entry: %s", entry.c_str());
				bypassList.push_back(ParseBypassEntry(entry));
			}
		}
	}

	// proxy bypass boolean
	bypassLocalNames = GetBoolFromDictionary(configDict.get(),
		kSCPropNetProxiesExcludeSimpleHostnames, false);

	initialized = true;
}

// Callback for CFNetworkExecuteProxyAutoConfigurationURL. client is a 
// pointer to a CFTypeRef. This stashes either error or proxies in that 
// location.
static void ResultCallback(void* client, CFArrayRef proxies, CFErrorRef error)
{
	CFTypeRef* resultPtr = (CFTypeRef*) client;

	if (error)
		*resultPtr = CFRetain(error);
	else
		*resultPtr = CFRetain(proxies);

	CFRunLoopStop(CFRunLoopGetCurrent());
}

void DoPACRequst(void* data)
{
	PACRequestInfo* info = static_cast<PACRequestInfo*>(data);

	// We don't need to work around <rdar://problem/5530166> because 
	// we know that our caller has called CFNetworkCopyProxiesForURL.

	// Despite the fact that CFNetworkExecuteProxyAutoConfigurationURL has 
	// neither a "Create" nor a "Copy" in the name, we are required to 
	// release the return CFRunLoopSourceRef <rdar://problem/5533931>.
	CFTypeRef result = NULL;
	CFStreamClientContext context = { 0, &result, NULL, NULL, NULL };
	CFRef<CFRunLoopSourceRef> runLoopSource(
		CFNetworkExecuteProxyAutoConfigurationURL(info->scriptURL, info->url,
			ResultCallback, &context));

	if (!runLoopSource.get())
	{
		GetLogger()->Error("Could not create run loop during PAC request!");
		return;
	}

	static CFStringRef  kPrivateRunLoopMode =
		CFSTR("org.appcelerator.TitaniumDesktopProxy");
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource.get(),
		kPrivateRunLoopMode);
	CFRunLoopRunInMode(kPrivateRunLoopMode, 1.0e10, false);
	CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource.get(),
		kPrivateRunLoopMode);

	// Once the runloop returns, we should have either an error result or a 
	// proxies array result.  Do the appropriate thing with that result.
	if (CFGetTypeID(result) == CFErrorGetTypeID())
	{
		string error = CFErrorToString((CFErrorRef) result);
		GetLogger()->Error("Failed PAC request: %s", error.c_str());
	}
	else if (CFGetTypeID(result) == CFArrayGetTypeID())
	{
		CFArrayRef resultArray = (CFArrayRef) result;
		CFArrayAppendArray(info->proxies, resultArray, 
			CFRangeMake(0, CFArrayGetCount(resultArray)));
	}
	else
	{
		GetLogger()->Error("PAC request timed out in run loop!");
	}

	// Retain was called on this value during ResultCallback.
	if (result)
		CFRelease(result);
}

// If there are PAC URLs in the incomming proxy list, these will be
// expanded into proxy entries via the CFNetwork APIs. Normal proxy
// entries will simply be passed through the resulting array of proxies.
static void ExpandPACProxies(CFURLRef url, CFArrayRef proxies,
	CFMutableArrayRef expandedProxies)
{
	// For each incoming proxy, if it's not a PAC-based proxy, just add the proxy
	// to the results array.  If it /is/ a PAC-based proxy, run the PAC script 
	// and append its results to the array.
	CFIndex proxyCount = CFArrayGetCount(proxies);
	for (CFIndex proxyIndex = 0; proxyIndex < proxyCount; proxyIndex++)
	{
		CFDictionaryRef thisProxy = (CFDictionaryRef) CFArrayGetValueAtIndex(
			proxies, proxyIndex);
		CFStringRef proxyType  = (CFStringRef) CFDictionaryGetValue(
			thisProxy, kCFProxyTypeKey);

		// If it's not a PAC proxy, just copy it across.
		if (!CFEqual(proxyType, kCFProxyTypeAutoConfigurationURL))
		{
			CFArrayAppendValue(expandedProxies, thisProxy);
		}
		else
		{
			PACRequestInfo* info = new PACRequestInfo();
			info->url = url;
			info->scriptURL = (CFURLRef) CFDictionaryGetValue(
				thisProxy, kCFProxyAutoConfigurationURLKey);
			info->proxies = expandedProxies;

			// TODO(mrobinson): This should eventually be done asynchronously.
			Poco::Thread t;
			t.start(&DoPACRequst, info);
			try
			{
				t.join(1000);
			}
			catch (...)
			{
				GetLogger()->Error("PAC request join timed out!");
			}
		}
	}
}

SharedProxy TryCFNetworkCopyProxiesForURL(const string& queryURL)
{
	CFRef<CFURLRef> cfurl(CFURLCreateWithBytes(NULL, 
		(const UInt8 *) queryURL.c_str(), queryURL.size(),
		 kCFStringEncodingUTF8, NULL));

	if (!cfurl.get())
	{
		GetLogger()->Error("Could not make CFURL from %s", queryURL.c_str());
		return 0;
	}

	// Get the default proxies dictionary from CF.
	CFRef<CFDictionaryRef> proxySettings(SCDynamicStoreCopyProxies(NULL));
	if (!proxySettings.get())
	{
		GetLogger()->Error("SCDynamicStoreCopyProxies returned NULL");
		return 0;
	}

	// Call CFNetworkCopyProxiesForURL to get the proxy list.  Then expand 
	// any PAC-based proxies.
	CFRef<CFArrayRef> proxies(CFNetworkCopyProxiesForURL(
		cfurl.get(), proxySettings.get()));
	if (!proxies.get())
	{
		GetLogger()->Error("CFNetworkCopyProxiesForURL returned NULL");
		return 0;
	}

	// If there are any PAC entries in our list of proxies, we're going to
	// need to use the CFNetwork APIs to execute the script at the URL. The
	// resulting proxies will be placed in the correct order in proxiesToTry.
	CFRef<CFMutableArrayRef> proxiesToTry(CFArrayCreateMutable(
		NULL, 0, &kCFTypeArrayCallBacks));
	if (!proxiesToTry.get())
	{
		GetLogger()->Error("Couldn't create empty proxiesToTry array!");
		return 0;
	}
	ExpandPACProxies(cfurl.get(), proxies.get(), proxiesToTry.get());

	// Now iterate through the array and choose the first decent looking proxy.
	// TODO(mrobinson): This should eventually return a list of proxies.
	// TODO(mrobinson): This should eventually return SOCKS proxies.
	CFIndex proxyCount = CFArrayGetCount(proxiesToTry.get());
	for (CFIndex proxyIndex = 0; proxyIndex < proxyCount; proxyIndex++)
	{
		CFDictionaryRef proxy = (CFDictionaryRef) CFArrayGetValueAtIndex(
			proxiesToTry.get(), proxyIndex);
		CFStringRef proxyType = (CFStringRef) GetValueFromDictionary(proxy,
			kCFProxyTypeKey, CFStringGetTypeID());

		if (CFEqual(proxyType, kCFProxyTypeNone))
		{
			return 0;
		}
		else if (CFEqual(proxyType, kCFProxyTypeHTTP))
		{
			return GetProxyServerFromDictionary("http", proxy,
				kCFProxyHostNameKey, kCFProxyPortNumberKey);
		}
		else if (CFEqual(proxyType, kCFProxyTypeHTTPS))
		{
			return GetProxyServerFromDictionary("https", proxy,
				kCFProxyHostNameKey, kCFProxyPortNumberKey);
		}
		else if (CFEqual(proxyType, kCFProxyTypeFTP))
		{
			return GetProxyServerFromDictionary("ftp", proxy,
				kCFProxyHostNameKey, kCFProxyPortNumberKey);
		}
		else if (CFEqual(proxyType, kCFProxyTypeSOCKS))
		{
			return GetProxyServerFromDictionary("socks", proxy,
				kCFProxyHostNameKey, kCFProxyPortNumberKey);
		}
	}

	return 0;
}

SharedProxy GetProxyForURLImpl(Poco::URI& uri)
{
	InitializeOSXProxyConfig();

	// If the URL matches global bypass conditions, use a direct connection.
	if (bypassLocalNames && uri.getHost().find(".") == string::npos)
		return 0;

	if (ShouldBypass(uri, bypassList))
		return 0;

	// Try the proxies set via the manual global settings.
	string scheme(uri.getScheme());
	if (scheme == "http" && httpProxy.get())
		return httpProxy;
	if (scheme == "https" && httpsProxy.get())
		return httpsProxy;
	if (scheme == "ftp" && ftpProxy.get())
		return ftpProxy;

	// Fallback to SOCKS proxy.
	if (socksProxy.get())
		return socksProxy;

	return TryCFNetworkCopyProxiesForURL(uri.toString());
}

}
}

