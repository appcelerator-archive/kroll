/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "common.h"
#include <vector>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <dirent.h>
#include "../shared.h"

namespace kroll
{
	bool isWritable(std::string& path)
	{
		int rc = access(path.c_str(), W_OK);
		return rc == 0;	
	}
	std::string getRuntimeBaseDir()
	{
		std::string uid = std::string(getenv("USER"));
		std::string p(INSTALL_PREFIX);
		p.append("/");
		p.append(PRODUCT_NAME);
		if (uid.find("root")!=std::string::npos || isWritable(p))
		{
			return p;
		}
		passwd *user = getpwnam(uid.c_str());
		char *home = user->pw_dir;
		std::string str(home);
		str.append("/");
		str.append(PRODUCT_NAME);
		return str;
	}
	bool isFile(std::string &str)
	{
		struct stat st;
		return (stat(str.c_str(),&st)==0) && S_ISREG(st.st_mode);
	}
	bool isDirectory(std::string &dir)
	{
		struct stat st;
		return (stat(dir.c_str(),&st)==0) && S_ISDIR(st.st_mode);
	}
	bool isRuntimeInstalled()
	{
		std::string thedir = getRuntimeBaseDir();
		return isDirectory(thedir);
	}

	#define MAX_PATH 255
	std::string getExecutableDirectory()
	{
		char szTmp[50];
		sprintf(szTmp,"/proc/%d/exe",getpid());
		char pbuf[MAX_PATH];
		int c = readlink(szTmp,pbuf,MAX_PATH);
		pbuf[c]='\0';
		std::string str(pbuf);
		size_t pos = str.rfind("/");
		if (pos==std::string::npos)
		{
			return str;
		}
		return str.substr(0,pos);
	}
	std::string findRuntime(int op, std::string& version)
	{
		std::string base = getRuntimeBaseDir();
		base.append("/runtime/linux");
		return findVersioned(base,op,version);
	}
	std::string findModule(std::string name, int op, std::string& version)
	{
		std::string base = getRuntimeBaseDir();
		base.append("/modules/");
		base.append(name);
		return findVersioned(base,op,version);
	}
}

