/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

namespace kroll
{
	std::string BootUtils::FindBundledModuleZip(
		std::string name,
		std::string version,
		std::string applicationDirectory)
	{
		std::string zipName;
		if (name != "runtime")
			zipName.append("module-");
		zipName.append(name);
		zipName.append("-");
		zipName.append(version);
		zipName.append(".zip");

		std::string zipLocation = FileUtils::Join(
			applicationDirectory.c_str(),
			"dist",
			zipName.c_str(),
			NULL);

		if (FileUtils::IsFile(zipLocation))
			return zipLocation;
		else
			return std::string();
	}
}
