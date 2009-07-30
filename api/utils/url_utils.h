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
	class KROLL_API URLUtils
	{
		public:
		/**
		 * Encodes a URI value
		 */
		static std::string EncodeURIComponent(std::string value);

		/**
		 * Decodes a URI value
		 */
		static std::string DecodeURIComponent(std::string value);

		/**
		 * Convert a file URL to an absolute path
		 */
		static std::string FileURLToPath(std::string url);

		/**
		 * Convert an path to a file URL
		 */
		static std::string PathToFileURL(std::string path);

// These functions are not available outside a Kroll application
#if defined(KROLL_HOST_EXPORT) || defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
		/**
		 * Normalize a URL. If this url is an app:// URL, ensure that it
		 * has the app id as the hostname
		 */
		static std::string NormalizeURL(std::string& url);

		/**
		 * Convert a URL to a path if it is an app://, ti:// or file://
		 * URL. If this URL cannot be converted to a path, return the original URL
		 */
		static std::string URLToPath(string& url);

		static std::string TiURLToPath(std::string& url);
		static std::string AppURLToPath(std::string& url);

		protected:
		static std::string NormalizeAppURL(std::string& url);
#endif
	};
}
#endif
