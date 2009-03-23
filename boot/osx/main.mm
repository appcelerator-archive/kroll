/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <iostream>
#import <sstream>
#import <cstring>
#import <cstdio>
#import <cstdlib>
#import <dlfcn.h>
#import "stdlib.h"
#import "file_utils.h"

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

using namespace kroll;

//
// these UUIDs should never change and uniquely identify a package type
//
#define DISTRIBUTION_URL "http://download.titaniumapp.com"
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"



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

#define OS_NAME "osx"

#define KR_FATAL_ERROR(msg) \
{ \
	NSApplicationLoad();\
	NSRunCriticalAlertPanel(@"Application Error", [NSString stringWithCString:msg], @"Quit", nil, nil); \
	exit(1); \
}

#ifndef MAX_PATH
#define MAX_PATH 512
#endif


std::string GetExecutablePath()
{
	NSString *bundle = [[NSBundle mainBundle] bundlePath];
	NSString *contents = [NSString stringWithFormat:@"%@/Contents",bundle];
	return std::string([contents UTF8String]);
}
std::string GetDirectory(std::string &path)
{
	size_t i = path.rfind("/");
	if (i != std::string::npos)
	{
		return path.substr(0,i);
	}
	return ".";
}
std::string GetModuleName(std::string &path)
{
	size_t i = path.rfind("/");
	if (i != std::string::npos)
	{
		size_t x = path.rfind("/",i-1);
		if (x != std::string::npos)
		{
			return path.substr(x+1,i-x-1);
		}
	}
	return path;
}
std::string FindManifest()
{
	std::string currentDir = GetExecutablePath();
	std::string manifest = FileUtils::Join(currentDir.c_str(),"manifest",NULL);
	std::cout << "manifest = " << manifest << std::endl;
	if (FileUtils::IsFile(manifest))
	{
		return manifest;
	}
	return std::string();
}
std::string FindModuleDir()
{
	std::string currentDir = GetExecutablePath();
	std::string modules = FileUtils::Join(currentDir.c_str(),"modules",NULL);
	if (FileUtils::IsDirectory(modules))
	{
		return modules;
	}
	std::string runtime = FileUtils::GetRuntimeBaseDirectory();
	return FileUtils::Join(runtime.c_str(),"modules","osx",NULL);
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
		
		// make sure we create our runtime directory
		kroll::FileUtils::CreateDirectory(runtimeBase);
		
		char *updatesite = getenv(BOOT_UPDATESITE_ENVNAME);
		std::string url;
		if (!updatesite)
		{
			url = DISTRIBUTION_URL;
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
//NOTE: for now we determine this at compile time -- in the future
//we might want to actually programmatically determine if running on
//64-bit processor or not...
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
					// this is information that will allow us to 
					// determine which module/runtime to give you from
					// the network distribution site
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
				std::string exec = kroll::FileUtils::Join(installerDir.c_str(),"Installer App.app","Contents","MacOS","Installer App",NULL);
				
				// paranoia check
				if (kroll::FileUtils::IsFile(exec))
				{
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
typedef int Executor(int argc, const char **argv);

static void myExecve(NSString *executable, NSArray *args, NSDictionary *environment)
{
    char **argv = (char **)calloc(sizeof(char *), [args count] + 1);
    char **env = (char **)calloc(sizeof(char *), [environment count] + 1);
    NSEnumerator *e = [args objectEnumerator];
    NSString *s;    int i = 0;
    while ((s = [e nextObject]))
	{
		argv[i++] = (char *) [s UTF8String];
	}        
    e = [environment keyEnumerator];
    i = 0;
    while ((s = [e nextObject]))
	{
        env[i++] = (char *) [[NSString stringWithFormat:@"%@=%@", s, [environment objectForKey:s]] UTF8String];
	}	
#ifdef DEBUG
    std::cout << "before exec: " << [executable fileSystemRepresentation] << std::endl;
#endif
    execve([executable fileSystemRepresentation], argv, env);
}

int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	std::string manifest = FindManifest();
	if (manifest.empty())
	{
		KR_FATAL_ERROR("Application packaging error. The application manifest was not found in the correct location.");
		[pool release];
		return __LINE__;
	}
	std::string homedir = GetExecutablePath();
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
		[pool release];
		return __LINE__;
	}
	// run the app installer if any missing modules/runtime or
	// version specs not met
	if (!RunAppInstallerIfNeeded(homedir,runtimePath,manifest,modules,moduleDirs,appname,appid,runtimeOverride,guid))
	{
		[pool release];
		return __LINE__;
	}
	
	int rc = 0;
	std::string localRuntime = FileUtils::Join(homedir.c_str(),"runtime",NULL);
	std::string runtimeBasedir = FileUtils::GetRuntimeBaseDirectory();
	std::string moduleLocalDir = FindModuleDir();
	std::string moduleBasedir = FileUtils::Join(runtimeBasedir.c_str(),"modules","osx",NULL);
	std::ostringstream moduleList;

	// we now need to resolve and load each module and dependencies
	std::vector<std::string>::iterator i = moduleDirs.begin();
	while (i!=moduleDirs.end())
	{
		std::string moduleDir = (*i++);
		std::string moduleName = GetModuleName(moduleDir);
		std::string localModule = FileUtils::Join(homedir.c_str(),"modules",moduleName.c_str(),NULL);
		moduleList << moduleDir << ":";
	}

	char *fork_flag = getenv("KR_FORK");
	if (!fork_flag)
	{
		std::string dypath = localRuntime;
		dypath+=":";
		dypath+=runtimePath;
		NSString *frameworkPath = [NSString stringWithCString:dypath.c_str()];
		NSString *executablePath = [[NSBundle mainBundle] executablePath];
		NSMutableDictionary *environment = [NSMutableDictionary dictionaryWithDictionary:[[NSProcessInfo processInfo] environment]]; 
		NSString *curpath = (NSString*)[environment objectForKey:@"DYLD_FRAMEWORK_PATH"];
		if (curpath)
		{
			frameworkPath = [NSString stringWithFormat:@"%@:%@",frameworkPath,curpath];
		}
		[environment setObject:frameworkPath forKey:@"DYLD_FRAMEWORK_PATH"];
		NSString *libpath = [NSString stringWithFormat:@"%s:%s:%s",runtimePath.c_str(),dypath.c_str(),moduleList.str().c_str()];
		NSString *curlibpath = (NSString*)[environment objectForKey:@"DYLD_LIBRARY_PATH"];
		if (curlibpath)
		{
			libpath = [NSString stringWithFormat:@"%@:%@",libpath,curlibpath];
		}
		[environment setObject:libpath forKey:@"DYLD_LIBRARY_PATH"];
		[environment setObject:@"YES" forKey:@"KR_FORK"];
		[environment setObject:@"YES" forKey:@"WEBKIT_UNSET_DYLD_FRAMEWORK_PATH"];
		[environment setObject:executablePath forKey:@"WebKitAppPath"];
		[environment setObject:[NSString stringWithCString:homedir.c_str()] forKey:@"KR_HOME"];
		[environment setObject:[NSString stringWithCString:runtimePath.c_str()] forKey:@"KR_RUNTIME"];
		[environment setObject:[NSString stringWithCString:moduleList.str().c_str()] forKey:@"KR_MODULES"];
		[environment setObject:[NSString stringWithCString:runtimeBasedir.c_str()] forKey:@"KR_RUNTIME_HOME"];
		[environment setObject:[NSString stringWithCString:guid.c_str()] forKey:@"KR_APP_GUID"];

		NSMutableArray *arguments = [NSMutableArray arrayWithObjects:executablePath, nil];
		while (*++argv)
		{
			[arguments addObject:[NSString stringWithUTF8String:*argv]];
		}
		myExecve(executablePath, arguments, environment);
		char *error = strerror(errno);    
		NSString *errorMessage = [NSString stringWithFormat:@"Launching application failed with the error '%s' (%d)", error, errno];
		KR_FATAL_ERROR([errorMessage UTF8String]);
	}
	else
	{
		if (argc > 1 && strcmp(argv[1],"--wait-for-debugger")==0)
		{
			std::cout << "Press enter after attaching debugger... your PID: " << getpid() << std::endl;
			fgetc(stdin);
		}
		unsetenv("KR_FORK");
		
		// now we need to load the host and get 'er booted
		std::string khost = FileUtils::Join(runtimePath.c_str(),"libkhost.dylib",NULL);

		if (!FileUtils::IsFile(khost))
		{
			char msg[MAX_PATH];
			sprintf(msg,"Couldn't find required file: %s",khost.c_str());
			KR_FATAL_ERROR(msg);
			return __LINE__;
		}

		void* lib = dlopen(khost.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!lib)
		{
			char msg[MAX_PATH];
			sprintf(msg,"Couldn't load file: %s, error: %s",khost.c_str(),dlerror());
			KR_FATAL_ERROR(msg);
			[pool release];
			return __LINE__;
		}
		Executor *executor = (Executor*)dlsym(lib, "Execute");
		if (!executor)
		{
			char msg[MAX_PATH];
			sprintf(msg,"Invalid entry point for: %s",khost.c_str());
			KR_FATAL_ERROR(msg);
			[pool release];
			return __LINE__;
		}

		rc = executor(argc,(const char**)argv);
	}
	#ifdef DEBUG
		std::cout << "return code: " << rc << std::endl;
	#endif
	[pool release];
	return rc;
}
