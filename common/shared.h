/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

/* NOTE: don't include shared.h directly - it is used to pull in
 *      common platform code within the common platform library
 */

#ifndef _SHARED_H_
#define _SHARED_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef OS_WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#define EqualTo 0
#define GreaterThanEqualTo 1
#define LessThanEqualTo 2
#define GreaterThan 3
#define LessThan 4

namespace kroll
{
	std::string trim(std::string str);
	void extractVersion(std::string spec, int *op, std::string &version);
	int makeVersion(std::string& ver);
	std::string findVersioned(std::string& path, int op, std::string& version);
	void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters);
	void readManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs);
	void listDir(std::string& path, std::vector<std::string>& files);

	extern bool isDirectory(std::string &dir);
	extern bool isFile(std::string &file);
	extern std::string findRuntime(int op, std::string& version);
	extern std::string findModule(std::string name, int op, std::string& version);

	#ifndef SHARED_INTERFACES

	std::string trim(std::string str)
	{
		std::string c(str);
		while (1)
		{
			size_t pos = c.rfind(" ");
			if (pos == std::string::npos || pos!=c.length()-1)
			{
				break;
			}
			c = c.substr(0,pos);
		}
		return c;
	}
	void extractVersion(std::string spec, int *op, std::string &version)
	{
		if (spec.find(">=")!=std::string::npos)
		{
			*op = GreaterThanEqualTo;
			version = spec.substr(2,spec.length());
		}
		else if (spec.find("<=")!=std::string::npos)
		{
			*op = LessThanEqualTo;
			version = spec.substr(2,spec.length());
		}
		else if (spec.find("<")!=std::string::npos)
		{
			*op = LessThan;
			version = spec.substr(1,spec.length());
		}
		else if (spec.find(">")!=std::string::npos)
		{
			*op = GreaterThan;
			version = spec.substr(1,spec.length());
		}
		else if (spec.find("=")!=std::string::npos)
		{
			*op = EqualTo;
			version = spec.substr(1,spec.length());
		}
		else
		{
			*op = EqualTo;
			version = spec;
		}
	}
	int makeVersion(std::string& ver)
	{
		std::string v;
		size_t pos = 0;
		while(1)
		{
			size_t newpos = ver.find(".",pos);
			if (newpos==std::string::npos)
			{
				v.append(ver.substr(pos,ver.length()));
				break;
			}
			v.append(ver.substr(pos,newpos));
			pos = newpos+1;
		}
		return atoi(v.c_str());
	}

	static bool compareVersions(std::string a, std::string b)
	{
		int a1 = makeVersion(a);
		int b1 = makeVersion(b);
		return b1 > a1;
	}

	std::string findVersioned(std::string& path, int op, std::string& version)
	{
		std::vector<std::string> files;
		std::vector<std::string> found;
		listDir(path,files);
		std::vector<std::string>::iterator iter = files.begin();
		int findVersion = makeVersion(version);
		while (iter!=files.end())
		{
			std::string str = (*iter++);
			std::string fullpath = std::string(path);
			fullpath.append(PATH_SEP);
			fullpath.append(str);
			if (isDirectory(fullpath))
			{
				int theVersion = makeVersion(str);
				bool matched = false;

				switch(op)
				{
					case EqualTo:
					{
						matched = (theVersion == findVersion);
						break;
					}
					case GreaterThanEqualTo:
					{
						matched = (theVersion >= findVersion);
						break;
					}
					case LessThanEqualTo:
					{
						matched = (theVersion <= findVersion);
						break;
					}
					case GreaterThan:
					{
						matched = (theVersion > findVersion);
						break;
					}
					case LessThan:
					{
						matched = (theVersion < findVersion);
						break;
					}
				}
				if (matched)
				{
					found.push_back(std::string(str));
				}
			}
		}
		if (found.size() > 0)
		{
			std::sort(found.begin(),found.end(),compareVersions);
			std::string file = found.at(0);
			std::string f = std::string(path);
			f.append(PATH_SEP);
			f.append(file);
			return f;
		}
		return std::string("");
	}

	void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters)
	{
		std::string::size_type lastPos = str.find_first_not_of(delimeters,0);
		std::string::size_type pos = str.find_first_of(delimeters,lastPos);
		while (std::string::npos!=pos || std::string::npos!=lastPos)
		{
			tokens.push_back(str.substr(lastPos,pos-lastPos));
			lastPos = str.find_first_not_of(delimeters,pos);
			pos = str.find_first_of(delimeters,lastPos);
		}
	}

	void readManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs)
	{
		std::ifstream file(path.c_str());
		while (!file.eof())
		{
			std::string line;
			std::getline(file,line);
			if (line.find("#")==0 || line.find(" ")==0)
			{
				continue;
			}
			size_t pos = line.find(":");
			if (pos!=std::string::npos)
			{
				std::string key = trim(line.substr(0,pos));
				std::string value = trim(line.substr(pos+1,line.length()));
				int op;
				std::string version;
				extractVersion(value,&op,version);
				if (key == "runtime")
				{
					runtimePath = findRuntime(op,version);
				}
				else
				{
					std::string dir = findModule(key,op,version);
					if (dir != "")
					{
						modules.push_back(key);
						moduleDirs.push_back(dir);
					}
				}
			}
		}
		file.close();
	}

	#if !defined(OS_WIN32)
	#include <sys/types.h>
	#include <dirent.h>
	#endif

	void listDir(std::string& path, std::vector<std::string> &files)
	{
	#if defined(OS_WIN32)

		WIN32_FIND_DATA findFileData;
		std::string q(path+"\\*");
		HANDLE hFind = FindFirstFile(q.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				files.push_back(std::string(findFileData.cFileName));
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}

	#else
		DIR *dp;
		struct dirent *dirp;
		if ((dp = opendir(path.c_str()))!=NULL)
		{
			while ((dirp = readdir(dp))!=NULL)
			{
				std::string fn = std::string(dirp->d_name);
				if (fn.substr(0,1)=="." || fn.substr(0,2)=="..")
				{
					continue;
				}
				files.push_back(fn);
			}
			closedir(dp);
		}
	#endif
	}	

#endif
}

#endif

