/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

namespace UTILS_NS
{
	vector<string>& BootUtils::GetComponentSearchPaths()
	{
		static bool initialized = false;
		if (!initialized)
		{
			// Allow the user to force an override to the runtime home by setting the
			// appropriate environment variable -- this will be the first path searched
			if (EnvironmentUtils::Has("KR_RUNTIME_HOME"))
				componentSearchPaths.push_back(EnvironmentUtils::Get("KR_RUNTIME_HOME"));

			// Kroll runtime and modules will located by searching the following paths in order:
			// 1. ~/.PRODUCT_NAME (eg. ~/.titanium)
			// 2. /opt/PRODUCT_NAME (default runtime base path for system-wide installation)
			// 3. /usr/local/lib/PRODUCT_NAME
			// 4. /usr/lib/PRODUCT_NAME
			string pname = PRODUCT_NAME;
			std::transform(pname.begin(), pname.end(), pname.begin(), tolower);
			componentSearchPaths.push_back(FileUtils::GetUserRuntimeHomeDirectory());
			componentSearchPaths.push_back(string("/opt/") + pname);
			componentSearchPaths.push_back(string("/usr/local/lib/") + pname);
			componentSearchPaths.push_back(string("/usr/lib/") + pname);

			initialized = true;
		}
		return componentSearchPaths;
	}
}
