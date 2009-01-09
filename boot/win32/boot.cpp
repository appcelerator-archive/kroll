/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <process.h>
#include <iostream>

#define KROLL_API 
#include "api/file_utils.h"

using namespace kroll;

#define alert(s) MessageBox(NULL,s,"DEBUG",MB_OK)

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

#ifdef WIN32_CONSOLE
int main (int argc, char **argv)
#else
int WinMain(HINSTANCE, HINSTANCE, LPSTR cl, int show)
#endif
{
	std::string cwd = getExecutableDirectory();
	std::string runtimeDir = getRuntimeBaseDir();
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

	if (!isRuntimeInstalled())
	{
		std::cout << "Runtime isn't installed" << std::endl;

		std::string installer = std::string(cwd);
		installer.append("\\installer");
		if (!isDirectory(installer))
		{
			fprintf(stderr,"invalid installation. installer path doesn't exist at: %s\n",installer.c_str());
			return __LINE__;
		}
		std::string binary = std::string(installer);
		binary.append("\\install.exe");
		if (!isFile(binary))
		{
			fprintf(stderr,"invalid installation. installer file doesn't exist at: %s\n",binary.c_str());
			return __LINE__;
		}
		std::string cmdline = std::string(binary);
		cmdline.append(" \"");
		cmdline.append(installer);
		cmdline.append("\" \"");
		cmdline.append(runtimeDir);
		cmdline.append("\"");

        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        si.cb = sizeof(si);

        std::cout << "Calling command line: " << cmdline << std::endl;
	    // launch our subprocess as a child of this process
	    // and wait for it to exit before we exit
        if (CreateProcessA(NULL,
						(LPSTR)cmdline.c_str(),
						0,
						0,
						false,
						CREATE_DEFAULT_ERROR_MODE,
						0,
						cwd.c_str(),
                        &si,
						&pi) != false)
		{
			WaitForSingleObject(pi.hProcess,INFINITE);
		}

		// clean up
		CloseHandle(pi.hProcess);
	}

	std::cout << "Runtime is installed, cwd=" << cwd <<  std::endl;

	// 1. read the application manifest to determine what's needed
	std::string c=std::string(cwd);
	c.append("\\manifest");
	std::vector<std::string> modules;
	std::vector<std::string> moduleDirs;
	std::string runtimePath;
	FileUtils::ReadManifest(c,runtimePath,modules,moduleDirs);

	std::cout << "Read manifest ? " << std::endl;

	// 2. setup the environment and get ready to launch our runtime
	std::string runtimeExec = std::string(runtimePath);
	runtimeExec.append("\\kernel.exe");

	std::string libPath = std::string(runtimePath);
	libPath.append(";");

	std::string moduleArgs;
	std::vector<std::string>::iterator iter = moduleDirs.begin();
	int a = 0;
	while(iter!=moduleDirs.end())
	{
		std::string module = (*iter++);
		libPath.append(module);
		libPath.append(";");
		moduleArgs.append(module);
		moduleArgs.append(";");
	}

	// we're going to cheat and set our local vars and then pull them back out
	libPath.append(";");
	libPath.append(runtimePath);
	libPath.append(";");
	libPath.append(runtimePath + "\\bin");

	SetEnvironmentVariable("PATH",libPath.c_str());
	SetEnvironmentVariable("KR_PLUGINS",moduleArgs.c_str());
	SetEnvironmentVariable("KR_HOME",cwd.c_str());
	SetEnvironmentVariable("KR_RUNTIME",runtimeDir.c_str());

	LPTCH env = GetEnvironmentStrings();

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    std::cout << "PATH=" << libPath << std::endl;
    std::cout << "launching runtime: " << runtimeExec << std::endl;

	// 3. launch our subprocess as a child of this process
	//    and wait for it to exit before we exit
    if (CreateProcessA(NULL,
						(LPSTR)runtimeExec.c_str(),
						0,
						0,
						false,
						CREATE_NEW_CONSOLE,
						env,
						runtimePath.c_str(),
                        &si,
						&pi) != false)
	{
		//we're running!
	}

	// clean up process
	CloseHandle(pi.hProcess);

	// free environment
	FreeEnvironmentStrings(env);

	return 0;
}

