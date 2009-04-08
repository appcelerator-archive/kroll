/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BOOT_UTILS_H_
#define _KR_BOOT_UTILS_H_

namespace kroll
{
	class KROLL_API BootUtils
	{
		public:
		static std::string FindBundledModuleZip(
			std::string name,
			std::string version,
			std::string applicationDirectory);
	};
}
#endif
