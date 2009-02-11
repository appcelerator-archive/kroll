/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

//////////////////////////////////////////////////////////
//
// ---------------
// The Kroll boot
// ---------------
// by Jeff Haynie, 1/31/09
//
// The boot process for the microkernel has several stages
// that it goes through to start a kroll container.
//
// It first needs to resolve all module dependencies for
// the applications by reading the manifest file in the
// application folder.  The manifest file has dependency
// related information for the application, namely what
// the runtime version must be and what modules are
// required and with what version.  The boot will attempt
// to locate the modules based on the Kroll runtime path
// and then if it cannot find one or more required components
// it will invoke an external installer process passing the
// necessary information to the installer to remotely fetch
// the components. Once the installer exists, either all
// dependencies have been met and installed, or not.  If
// not, the boot will exit (assuming the installer handled
// the error display to the user).  If it has, it will
// then build the appropriate runtime environment based on
// the resolved modules path and fork the current process
// handing control to the kernel with the new environment
// which replaces the executing process (but copying threads,
// memory, signals, etc).  NOTE: this is slightly different
// but conceptually almost the same in Win32.  The kernel
// is now in charge and will continue loading the modules
// from the correct path.
//
// You might think you could just dynamically load modules
// from here once they have been resolved.  Nice try. It
// doesn't work that way since the dynamic loader records the
// (DY)LD_LIBRARY_PATH (PATH on win32) before handing
// control to the main entrypoint.  Even if you set the
// environment in main, it will be ignored since the dynamic
// loader has already resolved libraries and setup the
// processes path.  So, we fork-and-replace our process to
// get around this.
//
// The boot is designed such that it requires no external
// dependencies - nada.  That's important and shouldn't get
// jacked up if you change it or the build file.  BE VERY
// CAREFUL which libraries you link against.  Don't link
// against any thirdparty libraries, including the Kroll API,
// etc.  The boot will be renamed to the name of the application
// process on application bundling, but it's an exact copy
// of what was built.  The boot should be as tiny as possible,
// have no depedencies, and provide minimal functionality to
// bootstrap the microkernel and handoff to the appropriate
// kernal runtime and modules.
//
//
//////////////////////////////////////////////////////////
#include "main.h"

typedef std::pair<std::string, std::string> StringPair;
static char **argv;
static int argc;

#ifdef OS_WIN32
std::string GetExecutablePath()
{
	char path[MAX_PATH];
	int size = GetModuleFileName(NULL,path,MAX_PATH);
	if (size>0)
	{
		path[size]='\0';
	}
	return std::string(path);
}
#endif


#ifdef OS_OSX
@interface TaskCallback : NSObject
+ (NSInvocation*) createInvocation: (SEL) selector;
- (BOOL) terminated: (NSNotification*) note;
@end

@implementation TaskCallback
+ (NSInvocation*) createInvocation: (SEL) selector {
  NSMethodSignature* signature = [self instanceMethodSignatureForSelector:selector];
  NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
  [invocation setSelector:selector];
  [invocation setTarget:[self alloc]];
  return invocation;
}
- (BOOL) terminated: (NSNotification*) note {
  [NSApp terminate:nil];
  return YES;
}
@end

@interface TaskInvoker : NSObject {
  	NSInvocation* terminateInvocation;
	NSTask *task;
}
-(void)terminate;
@end

@implementation TaskInvoker
- (id)initWithApplication:(NSString*)app environment:(NSDictionary*)env arguments:(NSArray*)args currentDirectory:(NSString*)dir {
	if ((self = [super init]))
	{
	    terminateInvocation = nil;
		task = [[NSTask alloc] init];
		[task setLaunchPath: app];
		if (env) [task setEnvironment: env];
		if (args) [task setArguments: args];
		if (dir) [task setCurrentDirectoryPath: dir];
  	}
  	return self;
}
- (void)dealloc{
	[task release];
	[super dealloc];
}
- (int)launch:(NSInvocation*) terminated{
  	terminateInvocation = terminated;

  	if (terminateInvocation != nil)
	{
    	[[NSNotificationCenter defaultCenter] addObserver:self
				    selector:@selector(taskTerminated:)
				    name:NSTaskDidTerminateNotification
				    object:nil];
  	}
	[task launch];
	[task waitUntilExit];
	return [task terminationStatus];
}
- (void)taskTerminated:(NSNotification *)note {
  	if (terminateInvocation != nil)
	{
		int terminationStatus = [task terminationStatus];
	    [terminateInvocation setArgument:&terminationStatus atIndex:2];
	    [terminateInvocation invoke];
	}
}
- (void)terminate
{
	[task terminate];
}
@end
static TaskInvoker* invoker = nil;

//
// this will capture console termination signals
// and cause the invoker to terminate
//
void termination(int)
{
	[invoker terminate];
    [NSApp terminate:nil];
}

#endif


std::string GetArgValue(int argc, char **argv, std::string name, std::string defaultValue)
{
	for (int c=1;c<argc;c++)
	{
		std::string arg(argv[c]);
		if (arg.find(name) == 0)
		{
			size_t i = arg.find("=");
			if (i!=std::string::npos)
			{
				return arg.substr(i+1);
			}
			return arg;
		}
	}
	return defaultValue;
}

bool RunAppInstallerIfNeeded(std::string &homedir,
						 std::string &runtimePath,
						 std::string &manifest,
						 std::vector< std::pair< std::pair<std::string,std::string>,bool> > &modules,
						 std::vector<std::string> &moduleDirs,
						 std::string &appname,
						 std::string &appid,
						 std::string &runtimeOverride)
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
		if (!kroll::FileUtils::IsDirectory(installerDir))
		{
			KR_FATAL_ERROR("Missing installer and application has modules that are not found.");
			return false;
		}

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

#ifdef OS_WIN32
		// in Win32 installer, we push the path to this process so he can
		// invoke back on us to do the unzip
		args.push_back(GetExecutablePath());
#endif

		// make sure we create our runtime directory
		kroll::FileUtils::CreateDirectory(runtimeBase);

		//FIXME - this needs to be compiled in!!
#ifdef OS_WIN32
		char updatesite[MAX_PATH];
		int size = GetEnvironmentVariable(BOOT_UPDATESITE_ENVNAME,(char*)&updatesite,MAX_PATH);
		updatesite[size]='\0';
#else
		const char *updatesite = getenv(BOOT_UPDATESITE_ENVNAME);
#endif
		std::string url;
		if (!updatesite)
		{
			const char *us = BOOT_UPDATESITE_URL;
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
			std::string sid = kroll::FileUtils::GetMachineId();
			std::string os = OS_NAME;
			std::string qs("?os="+os+"&sid="+sid+"&aid="+appid);
			std::vector< std::pair<std::string,std::string> >::iterator iter = missing.begin();
			int missingCount = 0;
			while (iter!=missing.end())
			{
				std::pair<std::string,std::string> p = (*iter++);
				std::string name;
				std::string path;
				bool found = false;
				if (p.first == "runtime")
				{
					name = "runtime-" + os + "-" + p.second;
					// see if we have a private runtime installed and we can link to that
					path = kroll::FileUtils::Join(installerDir.c_str(),"runtime",NULL);
					if (kroll::FileUtils::IsDirectory(path))
					{
						found = true;
						runtimePath = path;
					}
				}
				else
				{
					name = "module-" + p.first + "-" + p.second;
					// see if we have a private module installed and we can link to that
					path = kroll::FileUtils::Join(installerDir.c_str(),"modules",p.first.c_str(),NULL);
					if (kroll::FileUtils::IsDirectory(path))
					{
						found = true;
					}
				}
				if (found)
				{
					moduleDirs.push_back(path);
				}
				else
				{
					std::string u(url);
					u+="/";
					u+=name;
					u+=".zip";
					u+=qs;
					args.push_back(u);
					missingCount++;
				}
			}

			// we have to check again in case the private module/runtime was
			// resolved inside the application folder
			if (missingCount>0)
			{
				// run the installer app which will fetch remote modules/runtime for us
#ifdef OS_OSX
				std::string exec = kroll::FileUtils::Join(installerDir.c_str(),"Installer App.app","Contents","MacOS","Installer App",NULL);
#elif OS_WIN32
				std::string exec = kroll::FileUtils::Join(installerDir.c_str(),"Installer.exe",NULL);
#elif OS_LINUX
				std::string exec = kroll::FileUtils::Join(installerDir.c_str(),"installer",NULL);
#endif
				// paranoia check
				if (kroll::FileUtils::IsFile(exec))
				{
					// run and wait for it to exit..
					kroll::FileUtils::RunAndWait(exec,args);

					modules.clear();
					moduleDirs.clear();
					bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid,runtimeOverride);
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
			KR_FATAL_ERROR("Missing installer and application has additional modules that are needed.");
		}
		
		// unlink the temporary directory
		kroll::FileUtils::DeleteDirectory(sourceTemp);
	}
	return result;
}

bool IsForkedProcess()
{
#ifdef OS_WIN32
	char e[2];
	int size = GetEnvironmentVariable("KR_BOOT_PROCESS",(char*)&e,2);
	e[size]='\0';
	return strcmp(e,"1") == 0;
#else
	const char *e = getenv("KR_BOOT_PROCESS");
	return e!=NULL && strcmp(e,"1")==0;
#endif
}

#if defined(OS_WIN32)
bool IsUnzipper(int argc, const char **argv)
{
	return !GetArgValue(argc,(char **)argv,"--tiunzip",std::string()).empty();
}
void Unzip(const char **argv)
{
	std::string src = std::string(argv[2]);
	std::string dest = std::string(argv[3]);
	kroll::FileUtils::Unzip(src,dest);
}
bool IsSelfExtractor()
{
	// skip check if we're running from self-extractor
	std::string check = GetArgValue(__argc,__argv,"--kselfextractor","");
	if (!check.empty()) return false;
	std::string path = GetExecutablePath();
	HZIP hzip = OpenZip(path.c_str(),0);
	if (hzip!=0)
	{
		CloseZip(hzip);
		return true;
	}
	return false;
}
void SelfExtract(std::string &dir)
{
	std::string zip = GetExecutablePath();
#ifdef DEBUG
	std::cout << "SelfExtracting: " << zip << std::endl;
#endif
	kroll::FileUtils::Unzip(zip,dir);
}
#endif

bool IsInstallRequired()
{
	// first check to make sure we're not running with the boot
	// start flag
	std::string check = GetArgValue(argc,argv,BOOT_HOME_FLAG,"");
	if (!check.empty()) return false;
	
	// check for a marker file named .installed in the application directory
	std::string homedir = GetArgValue(argc,argv,BOOT_HOME_FLAG,kroll::FileUtils::GetApplicationDirectory());
	std::string marker = kroll::FileUtils::Join(homedir.c_str(),".installed",NULL);

	return !kroll::FileUtils::IsFile(marker);
}

#if defined(OS_WIN32)
typedef int Executor(HINSTANCE hInstance, int argc, const char **argv);
#else
typedef int Executor(int argc, const char **argv);
#endif

int Boot(int argc, char** argv)
{
#if defined(OS_WIN32)
	char home[MAX_PATH];
	int size = GetEnvironmentVariable("KR_RUNTIME",(char*)&home,MAX_PATH);
	home[size]='\0';
	std::string h(home);
	std::string path = kroll::FileUtils::Join(h.c_str(),"khost.dll",NULL);
	// unset before we launch in case this process launches 
	// another kroll process we don't want it to think we're already loaded
	SetEnvironmentVariable("KR_BOOT_PROCESS",NULL);
#ifdef DEBUG
	std::cout << "Boot attempting to load: " << path << std::endl;
#endif

	HMODULE dll = LoadLibraryA(path.c_str());

	if (!dll)
	{
		std::ostringstream msg;
		msg << "Error loading module: " << path << ", Error: " << GetLastError();
		KR_FATAL_ERROR(msg.str().c_str());
		return 1;
	}
	Executor *executor = (Executor*)GetProcAddress(dll, "Execute");
	if (!executor)
	{
		std::ostringstream msg;
		msg << "Invalid entry point for " << path;
		KR_FATAL_ERROR(msg.str().c_str());
		return 1;
	}
	return executor(::GetModuleHandle(NULL), __argc,(const char**)__argv);
#else
	const char *home = getenv("KR_RUNTIME");
	if (home == NULL)
	{
		KR_FATAL_ERROR("Cannot find KR_RUNTIME environment variable");
	}
#ifdef OS_OSX
	std::string path = kroll::FileUtils::Join((char*)home,"libkhost.dylib",NULL);
#else
	std::string path = kroll::FileUtils::Join((char*)home,"libkhost.so",NULL);
#endif
	// unset before we launch in case this process launches 
	// another kroll process we don't want it to think we're already loaded
	unsetenv("KR_BOOT_PROCESS");
	void* lib = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (!lib)
	{
		std::ostringstream msg;
		msg << "Error loading module: " << path << ", Error: " << dlerror();
		KR_FATAL_ERROR(msg.str().c_str());
		return 1;
	}
	Executor* executor = (Executor*)dlsym(lib, "Execute");
	if (!executor)
	{
		std::ostringstream msg;
		msg << "Invalid entry point for " <<  path;
		KR_FATAL_ERROR(msg.str().c_str());
		return 1;
	}
	return executor(argc,(const char**)argv);
#endif
}

int ForkProcess(std::string &exec, std::string &manifest, std::string &homedir, std::string &runtimeOverride)
{
#ifdef DEBUG
	std::cout << "ForkProcess - exec=" << exec << std::endl;
	std::cout << "ForkProcess - manifest=" << manifest << std::endl;
	std::cout << "ForkProcess - homedir=" << homedir << std::endl;
	std::cout << "ForkProcess - runtimeOverride=" << runtimeOverride << std::endl;
#endif

	int rc = 0;
	// ok, we have a manifest, now do some resolving
	std::vector< std::pair< std::pair<std::string,std::string>,bool> > modules;
	std::vector<std::string> moduleDirs;
	std::string runtimePath;
	std::string appname;
	std::string appid;
	bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid,runtimeOverride);
	if (!success)
	{
		rc = __LINE__;
	}
	else
	{
		// run the app installer if any missing modules/runtime or
		// version specs not met
		if (!RunAppInstallerIfNeeded(homedir,runtimePath,manifest,modules,moduleDirs,appname,appid,runtimeOverride))
		{
			rc = __LINE__;
		}
		else
		{
			std::string backup_homedir = homedir;
			while(1)
			{
				std::vector< std::pair<std::string,std::string> > addToEnvironment;
				if (IsInstallRequired())
				{
					// in this case, we need to change the path to the installer
					// override some ENV switches and continue
					std::string installer = kroll::FileUtils::Join(runtimePath.c_str(),"appinstaller",NULL);
					if (kroll::FileUtils::IsDirectory(installer))
					{
#ifdef DEBUG
						std::cout << "Detected a new app that needs installation. Loading installer from " << installer << std::endl;
#endif					
						// in the case there is no directory, just assume they don't want
						// to have an app installer and continue booting...
						addToEnvironment.push_back(std::pair<std::string,std::string>("KR_APP_INSTALL_FROM",homedir));
						// change the directory to load the app from to be our installer
						homedir = installer;
						//TODO: do we need to also override modules in case an app isn't using some of the 
						//modules that the installer needs?
					}
#ifdef DEBUG
					else
					{
						std::cout << "Detected a new app that needs installation, but no installer found in " << installer << std::endl;
					}
#endif
				}
#ifdef DEBUG
				std::cout << "Runtime Directory = " << runtimePath << std::endl;
#endif
				// now we should be able to just load the host
				std::stringstream dylib;
#ifdef OS_OSX
				const char *cd = getenv("DYLD_LIBRARY_PATH");
				if (cd) dylib << cd << ":";
#elif OS_LINUX
				dylib << "LD_LIBRARY_PATH=";
				const char *cd = getenv("LD_LIBRARY_PATH");
				if (cd) dylib << cd << ":";
#elif OS_WIN32
				char path[MAX_PATH];
				int bufsize = MAX_PATH;
				bufsize = GetEnvironmentVariable("PATH",(char*)&path,bufsize);
				if (bufsize > 0)
				{
					path[bufsize]='\0';
				}
				dylib << path << ";";
#endif
				dylib << runtimePath << KR_LIB_SEP;
				//TODO: we need to refactor this out since these are Titanium specific
				//for now, it doesn't hurt if you don't have them
#ifdef OS_OSX
				dylib << [[NSString stringWithCString:runtimePath.c_str()] fileSystemRepresentation] << ":";
				dylib << kroll::FileUtils::Join(runtimePath.c_str(),"WebKit.framework","Versions","Current",NULL) << ":";
				dylib << kroll::FileUtils::Join(runtimePath.c_str(),"WebCore.framework","Versions","Current",NULL) << ":";
				dylib << kroll::FileUtils::Join(runtimePath.c_str(),"JavaScriptCore.framework","Versions","Current",NULL) << ":";
#endif
#ifdef DEBUG
				std::cout << "library: " << dylib.str() << std::endl;
#endif
				std::stringstream runtimeEnv;
#ifdef OS_LINUX
				runtimeEnv << "KR_RUNTIME=" << runtimePath;
#else
				runtimeEnv << runtimePath;
#endif
#ifdef DEBUG
				std::cout << "runtime: " << runtimeEnv.str() << std::endl;
#endif
				std::stringstream runtimeHomeEnv;
				std::string runtimeBase = kroll::FileUtils::GetRuntimeBaseDirectory();
#ifdef OS_LINUX
				runtimeHomeEnv << "KR_RUNTIME_HOME=" << runtimeBase;
#else
				runtimeHomeEnv << runtimeBase;
#endif
#ifdef DEBUG
				std::cout << "runtimeHomeEnv: " << runtimeHomeEnv.str() << std::endl;
#endif
				std::stringstream home;
#ifdef OS_LINUX
				home << "KR_HOME=" << homedir;
#else
				home << homedir;
#endif
#ifdef DEBUG
				std::cout << "home: " << home.str() << std::endl;
#endif
				std::stringstream modules;
#ifdef OS_LINUX
				modules << "KR_MODULES=";
#endif
				std::vector<std::string>::iterator i = moduleDirs.begin();
				while(i!=moduleDirs.end())
				{
					std::string dir = (*i++);
					modules << dir << KR_LIB_SEP;
					dylib << dir << KR_LIB_SEP;
				}
#ifdef DEBUG
				std::cout << "modules: " << modules.str() << std::endl;
				std::cout << "exec: " << exec << std::endl;
#endif

#ifdef OS_WIN32
				SetEnvironmentVariable("PATH",dylib.str().c_str());
				SetEnvironmentVariable("KR_RUNTIME",runtimeEnv.str().c_str());
				SetEnvironmentVariable("KR_HOME",home.str().c_str());
				SetEnvironmentVariable("KR_MODULES",modules.str().c_str());
				SetEnvironmentVariable("KR_RUNTIME_HOME",runtimeHomeEnv.str().c_str());
				SetEnvironmentVariable("KR_BOOT_PROCESS","1");

				if (addToEnvironment.size()>0)
				{
					std::vector< std::pair<std::string,std::string> >::iterator i = addToEnvironment.begin();
					while(i!=addToEnvironment.end())
					{
						std::pair<std::string,std::string> entry = (*i++);
						SetEnvironmentVariable(entry.first.c_str(),entry.second.c_str());
					}
				}

				std::ostringstream ose;

				char *env = GetEnvironmentStrings();
				LPTSTR penv = env;
				int count = 0;
				while (true)
				{
	               if (*penv == 0) break;
	               while (*penv != 0) penv++;
	               penv++;
	               count++;
				}

				char **childEnv = (char**)alloca(sizeof(char *) * count+1);
				for (int i = 0; i < count; i++)
				{
	              ose << env << "\n";
				  childEnv[i] = env;
	              while(*env != '\0')
	                 env++;
	              env++;
				}
				childEnv[count] = NULL;

				char **childArgv = (char**)alloca(sizeof(char *) * __argc+1);
	//			childArgv[0]=(char*)exec.str().c_str();
				childArgv[0]=__argv[0];
				for (int c=1;c<__argc;c++)
				{
					childArgv[c] = strdup((char*)__argv[c]);
				}
				childArgv[__argc] = NULL;

	//			rc = _spawnvpe( _P_OVERLAY, exec.str().c_str(), childArgv, childEnv );
				rc = _spawnvpe( _P_OVERLAY, __argv[0], childArgv, childEnv );
#elif OS_LINUX
				const char** childArgv = (const char**) alloca(sizeof(char *) * (argc + 1));
				for (int c = 0; c < argc; c++)
				{
				        childArgv[c] = argv[c];
				}
				childArgv[argc] = NULL;

				// Copy all current environment variables
				extern char** environ;
				int e = 0;
				while (environ[e] != NULL)
				{
					addToEnvironment.push_back(StringPair(environ[e++], ""));
				}

				// Add our own environment variables
				std::string em = std::string("");
				std::string dylib_str = dylib.str();
				std::string runtimeEnv_str = runtimeEnv.str();
				std::string home_str = home.str();
				std::string modules_str = modules.str();
				std::string runtimeHomeEnv_str = runtimeHomeEnv.str();
				addToEnvironment.push_back(StringPair(dylib_str, em));
				addToEnvironment.push_back(StringPair(runtimeEnv_str, em));
				addToEnvironment.push_back(StringPair(home_str, em));
				addToEnvironment.push_back(StringPair(modules_str, em));
				addToEnvironment.push_back(StringPair(runtimeHomeEnv_str, em));
				addToEnvironment.push_back(StringPair(std::string("KR_BOOT_PROCESS"), std::string("1")));

				// Allocate this memory on the stack
				int env_size = addToEnvironment.size() + 1;
				const char** env = (const char**) alloca (sizeof(char*) * (env_size));

				// Convert our environment variable vector to a char** array
				for (size_t i = 0; i < addToEnvironment.size(); i++)
				{
					StringPair& pair = addToEnvironment.at(i);
					std::string line = pair.first;
					if (pair.second != "")
					{
						line.append("=");
						line.append(pair.second);
					}
					env[i] = line.c_str();
				}
				env[env_size] = NULL;

				int result = execve(exec.c_str(), (char* const*) childArgv, (char* const*) env);
				if (result < 0)
				{
					perror("execve");
					rc = __LINE__;
				}
#else
				invoker = [TaskInvoker alloc];
				NSInvocation* terminateInvocation = [TaskCallback createInvocation: @selector(terminated:)];

				// setup termination handlers that will ensure that we don't
				// orphan our subprocess
				signal(SIGHUP, &termination);
				signal(SIGTERM, &termination);
				signal(SIGINT, &termination);
				signal(SIGKILL, &termination);

				// create our program args (just pass what was passed to us)
				NSMutableArray *a = [[[NSMutableArray alloc] init] autorelease];
				for (int c=1;c<argc;c++)
				{
					[a addObject:[NSString stringWithFormat:@"%s",argv[c]]];
				}

				NSMutableDictionary *e = [[[NSMutableDictionary alloc] init] autorelease];
				[e setValue:[NSString stringWithCString:home.str().c_str()] forKey:@"KR_HOME"];
				[e setValue:[NSString stringWithCString:runtimeEnv.str().c_str()] forKey:@"KR_RUNTIME"];
				[e setValue:[NSString stringWithCString:runtimeHomeEnv.str().c_str()] forKey:@"KR_RUNTIME_HOME"];
				[e setValue:[NSString stringWithCString:modules.str().c_str()] forKey:@"KR_MODULES"];
				[e setValue:[NSString stringWithCString:dylib.str().c_str()] forKey:@"DYLD_LIBRARY_PATH"];
				[e setValue:@"1" forKey:@"KR_BOOT_PROCESS"];

				if (addToEnvironment.size()>0)
				{
					std::vector< std::pair<std::string,std::string> >::iterator i = addToEnvironment.begin();
					while(i!=addToEnvironment.end())
					{
						std::pair<std::string,std::string> entry = (*i++);
						[e setValue:[NSString stringWithCString:entry.second.c_str()] forKey:[NSString stringWithCString:entry.first.c_str()]];
					}
				}

				[invoker initWithApplication:[NSString stringWithCString:exec.c_str()] environment:e arguments:a currentDirectory:[[NSBundle mainBundle] bundlePath]];
				rc = [invoker launch:terminateInvocation];
				invoker = nil;
			
				if (rc == 253) // this is a restart request, loop to re-run
				{
#ifdef DEBUG
					std::cout << "Special exit with restart detected, here we go again ..." << std::endl;
#endif
					// we need to reset our home directory in case
					// it was changed during launch (like app installer)
					homedir = backup_homedir;
					rc = 0;
					continue;
				}
				// if we get anything else, break and exit
				break;
#endif
			}
		}
	}
#ifdef DEBUG
	std::cout << "ForkProcess returning exit code = " << rc << std::endl;
#endif
	return rc;
}

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
int WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int _argc, const char* _argv[])
#endif
{
	int rc = 0;
#if defined(OS_OSX)
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

#if defined(OS_WIN32)
	#if !defined(WIN32_CONSOLE)
		argc = __argc;
		argv = __argv;
	#else
		argc = _argc;
		argv = (char **)_argv;
	#endif
#else
	argc = _argc;
	argv = (char **)_argv;
#endif

	bool forkedProcess = IsForkedProcess();

#if defined(OS_LINUX)
	if (forkedProcess)
	{
		gtk_init(&argc, &argv);
	}
#endif

#if defined(OS_WIN32)
	// win32 is special .... since unzip isn't built-in to
	// .NET, we are going to just use the bundled libraries
	// for zlib in C++ to do it.  so, the C# installer will
	// call back into this process in Win32 to have us to
	// the unzip crap
	if (!forkedProcess && IsUnzipper(argc,(const char **)argv))
	{
#ifdef DEBUG
		std::cout << "Unzip request from installer ..." << std::endl;
#endif
		Unzip((const char **)argv);
		return 0;
	}
	if (!forkedProcess && IsSelfExtractor())
	{
		std::string tmpdir = kroll::FileUtils::GetTempDirectory();
#ifdef DEBUG
		std::cout << "Looks like a self-extracting executable ... extract to " << tmpdir << std::endl;
#endif
		SelfExtract(tmpdir);
		std::string src = GetExecutablePath();
 		std::string dest = kroll::FileUtils::Join(tmpdir.c_str(),"_installer.exe",NULL);
#ifdef DEBUG
		std::cout << "source=" << src << std::endl;
		std::cout << "dest=" << dest << std::endl;
#endif
		std::string manifest = kroll::FileUtils::Join(tmpdir.c_str(),"manifest",NULL);
		if (!kroll::FileUtils::IsFile(manifest))
		{
			KR_FATAL_ERROR("Error loading manifest. Your application is not properly configured or packaged.");
			rc = __LINE__;
		}
		else
		{
			rc = ForkProcess(src,manifest,tmpdir,tmpdir);
#ifdef DEBUG
			std::cout << "ran " << dest << " and exited with: " << rc << std::endl;
#endif
		}
		kroll::FileUtils::DeleteDirectory(tmpdir);
		return rc;
	}
#endif

	if (forkedProcess)
	{
		// we're inside the fork, boot
		rc = Boot(argc,argv);
	}
	else
	{
		// read the application manifest to determine what's needed
		std::string homedir = GetArgValue(argc,argv,BOOT_HOME_FLAG,kroll::FileUtils::GetApplicationDirectory());
		std::string runtimeOverride = GetArgValue(argc,argv,BOOT_RUNTIME_FLAG,homedir);
		std::string manifest = kroll::FileUtils::Join(homedir.c_str(),"manifest",NULL);
#ifdef DEBUG
		std::cout << "KR_HOME = " << homedir << std::endl;
		std::cout << "KR RUNTIME OVERRIDE= " << runtimeOverride << std::endl;
		std::cout << "KR MANIFEST = " << manifest << std::endl;
#endif

		if (!kroll::FileUtils::IsFile(manifest))
		{
			// oops! no manifest, looks like a packaging problem
			KR_FATAL_ERROR("Error loading manifest. Your application is not properly configured or packaged.");
			rc = __LINE__;
		}
		else
		{
#ifdef OS_WIN32
			std::string exec = GetExecutablePath();
#elif OS_OSX
			NSString* e = [[NSBundle mainBundle] executablePath];
			std::string exec = [e UTF8String];
#else
			std::string exec = argv[0];
#endif
			rc = ForkProcess(exec,manifest,homedir,runtimeOverride);
		}
	}

#if defined(OS_OSX)
	[pool release];
#endif
	return rc;
}
