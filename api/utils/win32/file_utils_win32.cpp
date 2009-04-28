/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

#include <windows.h>
#include <shlobj.h>
#include <Iphlpapi.h>
#include <process.h>

namespace UTILS_NS
{
	std::string FileUtils::GetUserRuntimeHomeDirectory()
	{
		char path[MAX_PATH];
		if (SHGetSpecialFolderPath(NULL, path, CSIDL_APPDATA, FALSE))
		{
			return Join(path, PRODUCT_NAME, NULL);
		}
		else
		{
			// Not good! What do we do in this case? I guess just use  a reasonable 
			// default for Windows. Ideally this should *never* happen.
			return Join("C:", PRODUCT_NAME, NULL);
		}
	}
	
	std::string FileUtils::GetSystemRuntimeHomeDirectory()
	{
		char path[MAX_PATH];
		if (SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_APPDATA, FALSE))
		{
			return Join(path, PRODUCT_NAME, NULL);
		}
		else
		{
			return GetUserRuntimeHomeDirectory();
		}
	}
	
	// TODO: implement this for other platforms
	void FileUtils::CopyRecursive(std::string &dir, std::string &dest)
	{
		if (!IsDirectory(dest))
		{
			CreateDirectory(dest);
		}
	
		std::cout << "\n>Recursive copy " << dir << " to " << dest << std::endl;
		WIN32_FIND_DATA findFileData;
		std::string q(dir+"\\*");
		HANDLE hFind = FindFirstFile(q.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::string filename = findFileData.cFileName;
				if (filename == "." || filename == "..") continue;
	
				std::string srcName = dir + "\\" + filename;
				std::string destName = dest + "\\" + filename;
	
				if (IsDirectory(srcName))
				{
					std::cout << "create dir: " << destName << std::endl;
					FileUtils::CreateDirectory(destName);
					CopyRecursive(srcName, destName);
				}
				else
				{
					//std::cout << "> copy file " << srcName << " to " << destName << std::endl;
					CopyFileA(srcName.c_str(), destName.c_str(), FALSE);
				}
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}
	}
}

