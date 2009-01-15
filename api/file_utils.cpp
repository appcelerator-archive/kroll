/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "file_utils.h"

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#elif defined(OS_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <process.h>
#endif

namespace kroll
{
	static bool CompareVersions(std::string a, std::string b)
	{
		int a1 = FileUtils::MakeVersion(a);
		int b1 = FileUtils::MakeVersion(b);
		return b1 > a1;
	}
	bool FileUtils::IsFile(std::string &file)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:file.c_str()] isDirectory:&isDir];
		return found && !isDir;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(file.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000000) == 0x00000000;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		struct stat st;
		return (stat(file.c_str(),&st)==0) && S_ISREG(st.st_mode);
#endif
	}

	bool FileUtils::IsDirectory(std::string &dir)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:dir.c_str()] isDirectory:&isDir];
		return found && isDir;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(dir.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000010) == 0x00000010;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		struct stat st;
		return (stat(dir.c_str(),&st)==0) && S_ISDIR(st.st_mode);
#endif
	}
	bool FileUtils::IsHidden(std::string &file)
	{
#ifdef OS_OSX
		// TODO finish this
		return false;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(file.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000002) == 0x00000002;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		return (file.size() > 0 && file.at(0) == '.');
#endif
	}
	bool FileUtils::IsRuntimeInstalled()
	{
		std::string dir = GetRuntimeBaseDirectory();
		return IsDirectory(dir);
	}
	std::string FileUtils::GetRuntimeBaseDirectory()
	{
#ifdef OS_WIN32
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
#elif OS_OSX
		// check to see if we already have a local one
		NSString *localDir = [[NSString stringWithFormat:@"~/Library/Application Support/%s",PRODUCT_NAME] stringByExpandingTildeInPath];
		std::string ls = std::string([localDir UTF8String]);
		if (IsDirectory(ls))
		{
			return std::string([localDir UTF8String]);
		}

		// first check to see if we can install in system directory by checking
		// if we can write to it
		NSString *systemPath = @"/Library/Application Support";
		if ([[NSFileManager defaultManager] isWritableFileAtPath:systemPath])
		{
			return std::string([[systemPath stringByAppendingString:@"/"PRODUCT_NAME] UTF8String]);
		}
		// if not, we fall back to installing into user directory
		return std::string([localDir UTF8String]);
#elif OS_LINUX
		std::string uid = std::string(getenv("USER"));
		std::string p(INSTALL_PREFIX);
		p.append("/");
		p.append(PRODUCT_NAME);
		bool writable = (access(p.c_str(),W_OK)) == 0;
		if (uid.find("root")!=std::string::npos || writable)
		{
			return p;
		}
		passwd *user = getpwnam(uid.c_str());
		char *home = user->pw_dir;
		std::string str(home);
		str.append("/");
		str.append(PRODUCT_NAME);
		return str;
#endif
	}
	std::string FileUtils::Trim(std::string str)
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
	void FileUtils::ExtractVersion(std::string& spec, int *op, std::string &version)
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
	int FileUtils::MakeVersion(std::string& ver)
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
	std::string FileUtils::FindVersioned(std::string& path, int op, std::string& version)
	{
		std::vector<std::string> files;
		std::vector<std::string> found;
		ListDir(path,files);
		std::vector<std::string>::iterator iter = files.begin();
		int findVersion = MakeVersion(version);
		while (iter!=files.end())
		{
			std::string str = (*iter++);
			std::string fullpath = std::string(path);
			fullpath.append(KR_PATH_SEP);
			fullpath.append(str);
			if (IsDirectory(fullpath))
			{
				int theVersion = MakeVersion(str);
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
			std::sort(found.begin(),found.end(),CompareVersions);
			std::string file = found.at(0);
			std::string f = std::string(path);
			f.append(KR_PATH_SEP);
			f.append(file);
			return f;
		}
		return std::string();
	}
	void FileUtils::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters)
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
	bool FileUtils::ReadManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs)
	{
		std::ifstream file(path.c_str());
		if (file.bad() || file.fail())
		{
			return false;
		}

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
				std::string key = Trim(line.substr(0,pos));
				std::string value = Trim(line.substr(pos+1,line.length()));
				int op;
				std::string version;
				ExtractVersion(value,&op,version);
				if (key == "runtime")
				{
					runtimePath = FindRuntime(op,version);
				}
				else
				{
					std::string dir = FindModule(key,op,version);
					if (dir != "")
					{
						modules.push_back(key);
						moduleDirs.push_back(dir);
					}
				}
			}
		}
		file.close();
		return true;
	}
	std::string FileUtils::FindRuntime(int op, std::string& version)
	{
		std::string runtime = GetRuntimeBaseDirectory();
		std::string path(runtime);
#ifdef OS_WIN32
		path += "\\runtime\\win32";
#elif OS_LINUX
		path += "/runtime/linux";
#elif OS_OSX
		path += "/runtime/osx";
#endif
		return FindVersioned(path,op,version);
	}
	std::string FileUtils::FindModule(std::string& name, int op, std::string& version)
	{
		std::string runtime = GetRuntimeBaseDirectory();
		std::string path(runtime);
#ifdef OS_WIN32
		path += "\\modules\\";
#else
		path += "/modules/";
#endif
		path += name;
		return FindVersioned(path,op,version);
	}
	void FileUtils::ListDir(std::string& path, std::vector<std::string> &files)
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
	int FileUtils::RunAndWait(std::string path, std::vector<std::string> args)
	{
#ifdef OS_OSX
		NSMutableArray *margs = [[[NSMutableArray alloc] init] autorelease];
		std::vector<std::string>::iterator i = args.begin();
		while (i!=args.end())
		{
			std::string arg = (*i++);
			[margs addObject:[NSString stringWithCString:arg.c_str()]];
		}
		NSTask *cmnd=[[NSTask alloc] init];
		[cmnd setLaunchPath:[NSString stringWithCString:path.c_str()]];
		[cmnd setArguments:margs];
		[cmnd launch];
		[cmnd waitUntilExit];
		int status = [cmnd terminationStatus];
		[cmnd release];
		cmnd = nil;
		return status;
#elif defined(OS_LINUX)
		std::string p(path);
		std::vector<std::string>::iterator i = args.begin();
		while (i!=args.end())
		{
			p+=" ";
			p+=(*i++);
		}
#ifdef DEBUG
		std::cout << "running: " << p << std::endl;
#endif
		return system(p.c_str());
#elif defined(OS_WIN32)
		const char **argv = new const char*[args.size()];
		std::vector<std::string>::iterator i = args.begin();
		int idx = 0;
		while (i!=args.end())
		{
			argv[idx++] = (*i++).c_str();
		}

		return _spawnvp(_P_WAIT, path.c_str(), argv);
#endif
	}
#ifndef NO_UNZIP
	void FileUtils::Unzip(std::string& source, std::string& destination)
	{
#ifdef OS_OSX
		//
		// we don't include gzip since we're on OSX
		// we just let the built-in OS handle extraction for us
		//
		std::vector<std::string> args;
		args.push_back("--noqtn");
		args.push_back("-x");
		args.push_back("-k");
		args.push_back("--rsrc");
		args.push_back(source);
		args.push_back(destination);
		RunAndWait(std::string("/usr/bin/ditto"),args);
#elif OS_LINUX
		std::vector<std::string> args;
		args.push_back("-qq");
		args.push_back(source);
		args.push_back("-d");
		args.push_back(destination);
		std::string cmdline("/usr/bin/unzip");
		RunAndWait(cmdline,args);
#elif OS_WIN32
		HZIP hz = OpenZip(source.c_str(),0);
		SetUnzipBaseDir(hz,destination.c_str());
		ZIPENTRY ze;
		GetZipItem(hz,-1,&ze);
		int numitems=ze.index;
		for (int zi=0; zi < numitems; zi++)
		{
			GetZipItem(hz,zi,&ze);
			UnzipItem(hz,zi,ze.name);
		}
		CloseZip(hz);
#endif
	}
#endif
}
