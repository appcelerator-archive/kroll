/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "stdlib.h"
#include <process.h>

#include "file_utils.h"

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

using namespace kroll;

//
// these flags are compiled in to allow them
// to be tailed to the embedded environment
//
#ifndef _BOOT_RUNTIME_FLAG
  #define _BOOT_RUNTIME_FLAG --kruntime
#endif

#ifndef _BOOT_HOME_FLAG
  #define _BOOT_HOME_FLAG --khome
#endif

#ifndef _BOOT_UPDATESITE_ENVNAME
  #define _BOOT_UPDATESITE_ENVNAME UPDATESITE
#endif

#ifndef BOOT_RUNTIME_FLAG
  #define BOOT_RUNTIME_FLAG STRING(_BOOT_RUNTIME_FLAG)
#endif

#ifndef BOOT_HOME_FLAG
  #define BOOT_HOME_FLAG STRING(_BOOT_HOME_FLAG)
#endif

#ifndef BOOT_UPDATESITE_ENVNAME
  #define BOOT_UPDATESITE_ENVNAME STRING(_BOOT_UPDATESITE_ENVNAME)
#endif


//
// these UUIDs should never change and uniquely identify a package type
//
#define DISTRIBUTION_URL "http://download.titaniumapp.com"
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"

#define OS_NAME "win32"

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#define KR_FATAL_ERROR(msg) \
{ \
	std::cerr << "Error: " << msg << std::endl; \
	MessageBox(NULL,msg,"Application Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL); \
}

std::string GetExecutablePath()
{
	char path[MAX_PATH];
	int size = GetModuleFileName(GetModuleHandle(NULL),(char*)path,MAX_PATH);
	if (size>0)
	{
		path[size]='\0';
	}
	return std::string(path);
}
std::string GetDirectory(std::string &path)
{
	size_t i = path.rfind("\\");
	if (i != std::string::npos)
	{
		return path.substr(0,i);
	}
	return ".";
}
std::string GetModuleName(std::string &path)
{
	size_t i = path.rfind("\\");
	if (i != std::string::npos)
	{
		size_t x = path.rfind("\\",i-1);
		if (x != std::string::npos)
		{
			return path.substr(x+1,i-x-1);
		}
	}
	return path;
}
std::string GetExecutableDir()
{
	std::string exec = GetExecutablePath();
	return GetDirectory(exec);
}
std::string FindManifest()
{
	std::string currentDir = GetExecutableDir();
	std::string manifest = FileUtils::Join(currentDir.c_str(),"manifest",NULL);
	if (FileUtils::IsFile(manifest))
	{
		return manifest;
	}
	return std::string();
}
std::string FindModuleDir()
{
	std::string currentDir = GetExecutableDir();
	std::string modules = FileUtils::Join(currentDir.c_str(),"modules",NULL);
	if (FileUtils::IsDirectory(modules))
	{
		return modules;
	}
	std::string runtime = FileUtils::GetRuntimeBaseDirectory();
	return FileUtils::Join(runtime.c_str(),"modules","win32",NULL);
}
bool RunAppInstallerIfNeeded(std::string &homedir,
						 std::string &runtimePath,
						 std::string &manifest,
						 std::vector< std::pair< std::pair<std::string,std::string>,bool> > &modules,
						 std::vector<std::string> &moduleDirs,
						 std::string &appname,
						 std::string &appid,
						 std::string &runtimeOverride,
						 std::string &guid)
{
	bool result = true;
	std::vector< std::pair<std::string,std::string> > missing;
	std::vector< std::pair< std::pair<std::string,std::string>, bool> >::iterator i = modules.begin();
	while(i!=modules.end())
	{
		std::pair< std::pair<std::string,std::string>,bool> p = (*i++);
		if (!p.second)
		{
			missing.push_back(p.first);
#ifdef DEBUG
			std::cout << "missing module: " << p.first.first << "/" << p.first.second <<std::endl;
#endif
		}
	}
	// this is where kroll should be installed
	std::string runtimeBase = kroll::FileUtils::GetRuntimeBaseDirectory();

	if (missing.size()>0)
	{
		// if we don't have an installer directory, just bail...
		std::string installerDir = kroll::FileUtils::Join(homedir.c_str(),"installer",NULL);

		std::string sourceTemp = kroll::FileUtils::GetTempDirectory();
		std::vector<std::string> args;
		// appname
		args.push_back(appname);
		// title
		//I18N: localize these
		args.push_back("Additional application files required");
		// message
		//I18N: localize these
		args.push_back("There are additional application files that are required for this application. These will be downloaded from the network. Please press Continue to download these files now to complete the installation of the application.");
		// extract directory
		args.push_back(sourceTemp);
		// runtime base
		args.push_back(runtimeBase);
		// in Win32 installer, we push the path to this process so he can
		// invoke back on us to do the unzip
		args.push_back(GetExecutablePath());

		// make sure we create our runtime directory
		kroll::FileUtils::CreateDirectory(runtimeBase);

		char updatesite[MAX_PATH];
		int size = GetEnvironmentVariable(BOOT_UPDATESITE_ENVNAME,(char*)&updatesite,MAX_PATH);
		updatesite[size]='\0';
		std::string url;
		if (size == 0)
		{
			const char *us = DISTRIBUTION_URL;
			if (strlen(us)>0)
			{
				url = std::string(us);
			}
		}
		else
		{
			url = std::string(updatesite);
		}

		if (!url.empty())
		{
			std::string mid = kroll::FileUtils::GetMachineId();
			std::string os = OS_NAME;
			std::string osver = kroll::FileUtils::EncodeURIComponent(kroll::FileUtils::GetOSVersion());
			char tiver[10];
			sprintf(tiver, "%.1f", PRODUCT_VERSION);
#ifdef OS_32
			std::string ostype = "32bit";
#else
			std::string ostype = "64bit";
#endif
			std::string qs("?os="+os+"&osver="+osver+"&tiver="+tiver+"&mid="+mid+"&aid="+appid+"&guid="+guid+"&ostype="+ostype);
			std::vector< std::pair<std::string,std::string> >::iterator iter = missing.begin();
			int missingCount = 0;
			while (iter!=missing.end())
			{
				std::pair<std::string,std::string> p = (*iter++);
				std::string uuid;
				std::string name = p.first;
				std::string version = p.second;
				std::string path;
				bool found = false;
				if (p.first == "runtime")
				{
					// see if we have a private runtime installed and we can link to that
					path = kroll::FileUtils::Join(installerDir.c_str(),"runtime",NULL);
					if (kroll::FileUtils::IsDirectory(path))
					{
							found = true;
							runtimePath = path;
					}
					else
					{
						uuid = RUNTIME_UUID;
					}
				}
				else
				{
					// see if we have a private module installed and we can link to that
					path = kroll::FileUtils::Join(installerDir.c_str(),"modules",p.first.c_str(),NULL);
					if (kroll::FileUtils::IsDirectory(path))
					{
						found = true;
					}
					else
					{
						uuid = MODULE_UUID;
					}
				}
				if (found)
				{
					moduleDirs.push_back(path);
				}
				else
				{
					std::string u(url);
					u+=qs;
					u+="&name=";
					u+=name;
					u+="&version=";
					u+=version;
					u+="&uuid=";
					u+=uuid;
#ifdef DEBUG
					std::cout << "Adding URL: " << u << std::endl;
#endif
					args.push_back(u);
					missingCount++;
				}
			}

			// we have to check again in case the private module/runtime was
			// resolved inside the application folder
			if (missingCount>0)
			{
				// run the installer app which will fetch remote modules/runtime for us
				std::string exec = kroll::FileUtils::Join(installerDir.c_str(),"Installer.exe",NULL);

				// paranoia check
				if (kroll::FileUtils::IsFile(exec))
				{
					// ensure temp directory exists
					kroll::FileUtils::CreateDirectory(sourceTemp);

					// run and wait for it to exit..
					kroll::FileUtils::RunAndWait(exec,args);

					modules.clear();
					moduleDirs.clear();
					bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid,runtimeOverride,guid);
					if (!success || modules.size()!=moduleDirs.size())
					{
						// must have failed
						// no need to error has installer probably was cancelled
						result = false;
					}
				}
				else
				{
					// something crazy happened
					result = false;
					KR_FATAL_ERROR("Missing installer and application has additional modules that are needed.");
				}
			}
		}
		else
		{
			result = false;
			KR_FATAL_ERROR("Missing installer and application has additional modules that are needed. Not updatesite has been configured.");
		}

		// unlink the temporary directory
		kroll::FileUtils::DeleteDirectory(sourceTemp);
	}
	return result;
}
typedef std::vector< std::pair< std::pair<std::string,std::string>,bool> > ModuleList;
typedef int Executor(HINSTANCE hInstance, int argc, const char **argv);

#define MAX_ENV_VARIABLE_SIZE 4096

int getEnv(std::string key, std::string &result)
{
	char buf[MAX_ENV_VARIABLE_SIZE];
	int size = GetEnvironmentVariable(key.c_str(),buf,MAX_ENV_VARIABLE_SIZE);
	if (size == 0) return 0;
	buf[size]='\0';
	result = std::string(buf);
	return size;
}

int PerformUnzip(char *from, char *to)
{
	std::string source(from);
	std::string destination(to);

	std::cout << "Performing Unzip " << source << " >>> " << destination << std::endl;
	kroll::FileUtils::Unzip(source, destination);

	return 0;
}

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int _argc, const char* _argv[])
#endif
{
#if !defined(WIN32_CONSOLE)
	int argc = __argc;
	char **argv = __argv;
#else
	int argc = _argc;
	char **argv = (char **)_argv;
#endif

	if (argc > 1 && strcmp(argv[1],"--tiunzip")==0)
	{
		if(argc != 4)
		{
			KR_FATAL_ERROR("Unable to perform unzip request.  3 arguments are expected --tiunzip <from> <to>");
			return __LINE__;
		}

		return PerformUnzip(argv[2], argv[3]);
	}

	std::string buf;
	int fork_flag = getEnv("KR_FORK",buf);
	int rc = 0;
	if (fork_flag == 0)
	{
		if (argc > 1 && strcmp(argv[1],"--wait-for-debugger")==0)
		{
			DebugBreak();
		}

		std::string manifest = FindManifest();
		if (manifest.empty())
		{
			KR_FATAL_ERROR("Application packaging error. The application manifest was not found in the correct location.");
			return __LINE__;
		}
		std::string homedir = GetExecutableDir();
		ModuleList modules;
		std::vector<std::string> moduleDirs;
		std::string runtimePath;
		std::string appname;
		std::string appid;
		std::string runtimeOverride = homedir;
		std::string guid;
		bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid,runtimeOverride,guid);
		if (!success)
		{
			return __LINE__;
		}
		// run the app installer if any missing modules/runtime or
		// version specs not met
		if (!RunAppInstallerIfNeeded(homedir,runtimePath,manifest,modules,moduleDirs,appname,appid,runtimeOverride,guid))
		{
			return __LINE__;
		}

		std::string localRuntime = FileUtils::Join(homedir.c_str(),"runtime",NULL);
		std::string runtimeBasedir = FileUtils::GetRuntimeBaseDirectory();
		std::string moduleLocalDir = FindModuleDir();
		std::string moduleBasedir = FileUtils::Join(runtimeBasedir.c_str(),"modules","win32",NULL);
		std::ostringstream moduleList;

		// we now need to resolve and load each module and dependencies
		std::vector<std::string>::iterator i = moduleDirs.begin();
		while (i!=moduleDirs.end())
		{
			std::string moduleDir = (*i++);
			std::string moduleName = GetModuleName(moduleDir);
			std::string localModule = FileUtils::Join(homedir.c_str(),"modules",moduleName.c_str(),NULL);
			moduleList << moduleDir << ";";
		}

		std::string dypath = localRuntime;
		dypath+=";";
		dypath+=runtimePath;
		std::string path;
		path+=dypath;
		path += moduleList.str();
		if (getEnv("PATH",buf) > 0)
		{
			path+=";";
			path+=buf;
		}

		std::cout << "\n\n\n**** SETTING PATH = " << path << "\n\n\n";

		SetEnvironmentVariable("KR_FORK","YES");
		SetEnvironmentVariable("KR_HOME",homedir.c_str());
		SetEnvironmentVariable("KR_RUNTIME",runtimePath.c_str());
		SetEnvironmentVariable("KR_MODULES",moduleList.str().c_str());
		SetEnvironmentVariable("KR_RUNTIME_HOME",runtimeBasedir.c_str());
		SetEnvironmentVariable("PATH",path.c_str());
		SetEnvironmentVariable("KR_APP_GUID",guid.c_str());

		_execvp(argv[0],argv);
	}
	else
	{
		SetEnvironmentVariable("KR_FORK",NULL);
		getEnv("KR_RUNTIME",buf);
		std::string runtimePath = buf;

		// now we need to load the host and get 'er booted
		std::string khost = FileUtils::Join(runtimePath.c_str(),"khost.dll",NULL);

		if (!FileUtils::IsFile(khost))
		{
			char msg[MAX_PATH];
			sprintf_s(msg,"Couldn't find required file: %s",khost.c_str());
			KR_FATAL_ERROR(msg);
			return __LINE__;
		}

		HMODULE dll = LoadLibrary(khost.c_str());
		if (!dll)
		{
			char msg[MAX_PATH];
			sprintf_s(msg,"Couldn't load file: %s, error: %d",khost.c_str(),GetLastError());
			KR_FATAL_ERROR(msg);
			return __LINE__;
		}
		Executor *executor = (Executor*)GetProcAddress(dll, "Execute");
		if (!executor)
		{
			char msg[MAX_PATH];
			sprintf_s(msg,"Invalid entry point for: %s",khost.c_str());
			KR_FATAL_ERROR(msg);
			return __LINE__;
		}

		rc = executor(::GetModuleHandle(NULL), argc,(const char**)argv);
	}

#ifdef DEBUG
	std::cout << "return code: " << rc << std::endl;
#endif
	return rc;
}
