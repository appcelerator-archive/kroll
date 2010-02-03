/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_URL_UTILS_H_
#define _KR_URL_UTILS_H_
#include <string>
namespace UTILS_NS
{
	namespace URLUtils
	{
		/**
		 * Encodes a URI value
		 */
		KROLL_API std::string EncodeURIComponent(std::string value);

		/**
		 * Decodes a URI value
		 */
		KROLL_API std::string DecodeURIComponent(std::string value);

		/**
		 * Convert a file URL to an absolute path
		 */
		KROLL_API std::string FileURLToPath(std::string url);

		/**
		 * Convert an path to a file URL
		 */
		KROLL_API std::string PathToFileURL(std::string path);

// These functions are not available outside a Kroll application
#if defined(KROLL_HOST_EXPORT) || defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
		/**
		 * Normalize a URL. If this url is an app:// URL, ensure that it
		 * has the app id as the hostname
		 */
		KROLL_API std::string NormalizeURL(const std::string& url);

		/**
		 * Convert a URL to a path if it is an app://, ti:// or file://
		 * URL. If this URL cannot be converted to a path, return the original URL
		 */
		KROLL_API std::string URLToPath(const string& url);

		/**
		 * Path portion of URL which is guauranteed to be a local and * blank file.
		 */
		KROLL_API std::string& BlankPageURL();

		KROLL_API std::string TiURLToPath(const std::string& url);
		KROLL_API std::string AppURLToPath(const std::string& url);
#endif
	};
}
#endif
