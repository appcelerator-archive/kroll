/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <windows.h>
#include <shlobj.h>
#include <api/base.h>
#include <string>
#include <vector>
#include "common.h"
#include "../shared.h"

namespace kroll
{
	std::string getRuntimeBaseDir()
	{
		char path[MAX_PATH];
		std::string dir;
		if (SHGetSpecialFolderPath(NULL,path,CSIDL_COMMON_APPDATA,FALSE))
		{
			dir.append(path);
			dir.append("\\");
			dir.append(PRODUCT_NAME);
		}
		else if (SHGetSpecialFolderPath(NULL,path,CSIDL_APPDATA,FALSE))
		{
			dir.append(path);
			dir.append("\\");
			dir.append(PRODUCT_NAME);
		}
		return dir;
	}
	bool isFile(std::string& str)
	{
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(str.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000000) == 0x00000000;
			FindClose(hFind);
			return yesno;
		}
		return false;
	}
	bool isDirectory(std::string& dir)
	{
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(dir.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000010) == 0x00000010;
			FindClose(hFind);
			return yesno;
		}
		return false;
	}
	bool isRuntimeInstalled()
	{
		std::string dir = getRuntimeBaseDir();
		return isDirectory(dir);
	}
	std::string getExecutableDirectory()
	{
		char path[MAX_PATH];
		GetModuleFileName(NULL,path,MAX_PATH);
		std::string p(path);
		std::string::size_type pos = p.rfind("\\");
		if (pos!=std::string::npos)
		{
			return p.substr(0,pos);
		}
		return p;
	}
	std::string findRuntime(int op, std::string& version)
	{
		std::string base = getRuntimeBaseDir();
		base.append("\\runtime\\win32");
		return findVersioned(base,op,version);
	}
	std::string findModule(std::string name, int op, std::string& version)
	{
		std::string base = getRuntimeBaseDir();
		base.append("\\modules\\");
		base.append(name);
		return findVersioned(base,op,version);
	}
}
