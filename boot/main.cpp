/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#if defined(OS_OSX)
#import <Cocoa/Cocoa.h>
#endif

#if defined(OS_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>
#include <api/file_utils.h>

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

#ifdef OS_OSX
  #define KR_FATAL_ERROR(msg) \
  { \
	[NSApplication sharedApplication]; \
	NSAlert *alert = [[NSAlert alloc] init]; \
	[alert addButtonWithTitle:@"OK"]; \
	[alert setMessageText:@"Application Error"]; \
	[alert setInformativeText:[NSString stringWithCString:msg]]; \
	[alert setAlertStyle:NSCriticalAlertStyle]; \
	[alert runModal]; \
	[alert release]; \
	 \
  }
#elif OS_WIN32
  #define KR_FATAL_ERROR(msg) \
  { \
	MessageBox(NULL,msg,"Application Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL); \
  }
#elif OS_LINUX
  #define KR_FATAL_ERROR(msg) \
  { \
	GtkWidget* dialog = gtk_message_dialog_new (NULL,  \
	                  GTK_DIALOG_MODAL, \
					  GTK_MESSAGE_ERROR,  \
	                  GTK_BUTTONS_OK, \
	                  msg); \
	gtk_dialog_run (GTK_DIALOG (dialog)); \
	gtk_widget_destroy (dialog); \
  }
#endif

bool RunAppInstallerIfNeeded(std::string &homedir,
						 std::string &runtimePath,
						 std::string &manifest, 
						 std::vector< std::pair< std::pair<std::string,std::string>,bool> > &modules, 
						 std::vector<std::string> &moduleDirs,
						 std::string &appname,
						 std::string &appid)
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
			std::cout << "missing module: " << p.first.first << "/" << p.first.second <<std::endl;
		}
	}
	// this is where kroll should be installed
	std::string runtimeBase = kroll::FileUtils::GetRuntimeBaseDirectory();
	
	if (missing.size()>0)
	{
		// if we don't have an installer directory, just bail...
		std::string installerDir = kroll::FileUtils::Join((char*)homedir.c_str(),"installer",NULL);
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
		
		// make sure we create our runtime directory
		kroll::FileUtils::CreateDirectory(runtimeBase);

		//FIXME - this needs to be compiled in!!
		const char *updatesite = getenv("UPDATESITE");
		std::string url;
		if (!updatesite)
		{
			//FIXME
//			url = "http://updatesite.titaniumapp.com/";

			url = "http://localhost/~";
			url+= kroll::FileUtils::GetUsername();
			url+="/titanium";
		}
		else
		{
			url = std::string(updatesite);
		}
		
		std::string sid = kroll::FileUtils::GetMachineId();
		std::string qs("?os=osx&sid="+sid+"&aid="+appid);
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
				name = "runtime-osx-" + p.second;
				// see if we have a private runtime installed and we can link to that
				path = kroll::FileUtils::Join((char*)installerDir.c_str(),"runtime",NULL);
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
				path = kroll::FileUtils::Join((char*)installerDir.c_str(),"modules",(char*)p.first.c_str(),NULL);
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
			std::string exec = kroll::FileUtils::Join((char*)installerDir.c_str(),"Installer App.app","Contents","MacOS","Installer App",NULL);
#elif OS_WIN32
			std::string exec = kroll::FileUtils::Join((char*)installerDir.c_str(),"installer.exe",NULL);
#elif OS_LINUX
			std::string exec = kroll::FileUtils::Join((char*)installerDir.c_str(),"installer",NULL);
#endif	
			// paranoia check
			if (kroll::FileUtils::IsFile(exec))
			{
				// run and wait for it to exit..
				kroll::FileUtils::RunAndWait(exec,args);

				modules.clear();
				moduleDirs.clear();
				bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid);
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
		// unlink the temporary directory
		kroll::FileUtils::DeleteDirectory(sourceTemp);
	}
	return result;
}

static pid_t child_pid = 0;
void termination(int term)
{
	if (child_pid>0)
	{
		 kill(child_pid, term);
	}
}

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
#include <windows.h>
int WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int argc, const char* argv[])
#endif
{
	int rc = 0;
#if defined(OS_OSX)
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

	// 1. read the application manifest to determine what's needed
	std::string homedir = kroll::FileUtils::GetApplicationDirectory();
	std::string manifest = kroll::FileUtils::Join((char*)homedir.c_str(),"manifest",NULL);
	std::cout << "manifest: " << manifest << std::endl;
	
	if (!kroll::FileUtils::IsFile(manifest))
	{
		// oops! no manifest, looks like a packaging problem
		KR_FATAL_ERROR("Error loading manifest. Your application is not properly configured or packaged.");
		rc = __LINE__;
	}
	else
	{
		// ok, we have a manifest, now do some resolving
		std::vector< std::pair< std::pair<std::string,std::string>,bool> > modules;
		std::vector<std::string> moduleDirs;
		std::string runtimePath;
		std::string appname;
		std::string appid;
		bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid);
		if (!success)
		{
			rc = __LINE__;
		}
		else
		{
			// run the app installer if any missing modules/runtime or 
			// version specs not met
			if (!RunAppInstallerIfNeeded(homedir,runtimePath,manifest,modules,moduleDirs,appname,appid))
			{
				rc = __LINE__;
			}
			else
			{
				// now we should be able to just load the host
				std::stringstream dylib;
#ifdef OS_OSX
				dylib << "DYLD_LIBRARY_PATH=";
#elif OS_LINUX
				dylib << "LD_LIBRARY_PATH=";
#elif OS_WIN32
				dylib << "PATH=";
#define BUFSIZE 1024
				char path[BUFSIZE];
				if (GetEnvironmentVariable("PATH",&path,BUFSIZE))
				{
					dylib << path << ";";
				}
#endif				
				//TODO: we need to refactor this out since these are Titanium specific
				//for now, it doesn't hurt if you don't have them
				dylib << [[NSString stringWithCString:runtimePath.c_str()] fileSystemRepresentation] << ":";
				dylib << kroll::FileUtils::Join((char*)runtimePath.c_str(),"WebKit.framework","Versions","Current",NULL) << ":";
				dylib << kroll::FileUtils::Join((char*)runtimePath.c_str(),"WebCore.framework","Versions","Current",NULL) << ":";
				dylib << kroll::FileUtils::Join((char*)runtimePath.c_str(),"JavaScriptCore.framework","Versions","Current",NULL) << ":";

				std::stringstream runtimeEnv;
				runtimeEnv << "KR_RUNTIME=" << runtimePath;
	
				std::stringstream runtimeHomeEnv;
				std::string runtimeBase = kroll::FileUtils::GetRuntimeBaseDirectory();
				runtimeHomeEnv << "KR_RUNTIME_HOME=" << runtimeBase;
				
				std::stringstream home;
				home << "KR_HOME=" << homedir;

				std::stringstream modules; // FIXME name
				modules << "KR_PLUGINS=";

				std::vector<std::string>::iterator i = moduleDirs.begin();
				while(i!=moduleDirs.end())
				{
					std::string dir = (*i++);
					modules << dir << KR_LIB_SEP;
				}
				
				std::stringstream exec;
#ifdef OS_WIN32
				exec << kroll::FileUtils::Join((char*)runtimePath.c_str(),"kkernel.exe",NULL);
#else
				exec << kroll::FileUtils::Join((char*)runtimePath.c_str(),"kkernel",NULL);
#endif
				

#ifdef OS_WIN32
				#error Complete this implementation for win32
#else
				char **childArgv = (char**)alloca(sizeof(char *) * (argc + 1));
				for (int c=0;c<argc;c++)
				{
					childArgv[c] = strdup((char*)argv[c]);
				}
				childArgv[argc] = NULL;
				
				char **env = (char **)alloca(sizeof(char *) * 6);
				env[0]=strdup(dylib.str().c_str());
				env[1] = (char*)strdup(runtimeEnv.str().c_str());
				env[2] = (char*)strdup(home.str().c_str());
				env[3] = (char*)strdup(modules.str().c_str());
				env[4] = (char*)strdup(runtimeHomeEnv.str().c_str());
				env[5] = NULL;

				int result = execve(exec.str().c_str(),(char* const*)childArgv,(char* const*)env);
				if (result < 0)
				{
					perror("execve");
					rc = __LINE__;
				}
#endif
			}
		}
	}

#if defined(OS_OSX)
	[pool release];
#endif
	return rc;
}

