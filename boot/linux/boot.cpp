/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <api/file_utils.h>

using namespace kroll;

std::string getExecutableDirectory()
{
	char tmp[100];
	sprintf(tmp,"/proc/%d/exe",getpid());
	char pbuf[255];
	int c = readlink(tmp,pbuf,255);
	pbuf[c]='\0';
	std::string str(pbuf);
	size_t pos = str.rfind("/");
	if (pos==std::string::npos) return str;
	return str.substr(0,pos);
}

int main(int argc, char* argv[], char* environ[])
{
	std::string cwd = getExecutableDirectory();
	std::string runtimeDir = FileUtils::GetRuntimeBaseDirectory();

std::cout << "cwd = " << cwd << std::endl;
std::cout << "runtime = " << runtimeDir << std::endl;

	if (!FileUtils::IsRuntimeInstalled())
	{
		std::string installer = std::string(cwd);
		installer.append("/installer");
		if (!FileUtils::IsDirectory(installer))
		{
			fprintf(stderr,"invalid installation. installer path doesn't exist at: %s\n",installer.c_str());
			return __LINE__;
		}	
		std::string binary = std::string(installer);
		binary.append("/kinstall");
		if (!FileUtils::IsFile(binary))
		{
			fprintf(stderr,"invalid installation. installer file doesn't exist at: %s\n",binary.c_str());
			return __LINE__;
		}
		std::string cmdline = std::string(binary);
		std::vector<std::string> args;
		args.push_back(installer);
		args.push_back(runtimeDir);
		FileUtils::RunAndWait(cmdline,args);
	}

	// 1. read the application manifest to determine what's needed
	std::string c=std::string(cwd);
	c.append("/manifest");
	std::vector<std::string> modules;
	std::vector<std::string> moduleDirs;
	std::string runtimePath;
	bool success = FileUtils::ReadManifest(c,runtimePath,modules,moduleDirs);
	if (!success)
	{
		std::cerr << "Could not read manifest: " << c << std::endl;
		return __LINE__;
	}

	// 2. setup the environment and get ready to launch our runtime
	std::string runtimeExec = std::string(runtimePath);
	runtimeExec.append("/kkernel");

	std::string libPath = std::string(runtimePath);
	libPath.append(":");
	libPath.append(runtimePath);
	libPath.append("/lib:");

	std::string moduleArgs;
	std::vector<std::string>::iterator iter = moduleDirs.begin();
	while(iter!=moduleDirs.end())
	{
		std::string module = (*iter++);
		libPath.append(module);
		libPath.append(":");
		moduleArgs.append(module);
		moduleArgs.append(":");
	}

	size_t environ_size = 0;
	while(*(environ + environ_size))
		environ_size++;

	// create our enviroment
	char **env=(char**)calloc(sizeof(char*),environ_size + 5);

	int cur_env;
	for (cur_env = 0; cur_env < (int) environ_size; cur_env++)
	{
		env[cur_env] = environ[cur_env];
	}

	std::string ldlib = std::string("LD_LIBRARY_PATH=");
	ldlib.append(libPath);
	env[cur_env + 0]=(char*)ldlib.c_str();

	std::string plug = std::string("KR_MODULES=");
	plug.append(moduleArgs);
	env[cur_env + 1]=(char*)plug.c_str();

	std::string home = std::string("KR_HOME=");
	home.append(cwd);
	env[cur_env + 2]=(char*)home.c_str();
	
	std::string rhome = std::string("KR_RUNTIME=");
	rhome.append(runtimeDir);
	env[cur_env + 3]=(char*)rhome.c_str();

	std::cout << "LD_LIBRARY_PATH=" << libPath << std::endl;
	std::cout << "KR_MODULES=" << moduleArgs << std::endl;
	std::cout << "KR_HOME=" << cwd << std::endl;
	std::cout << "KR_RUNTIME=" << runtimeDir << std::endl;
	
   int rc = execve(runtimeExec.c_str(),argv,env);
	if (rc < 0)
	{
		// failed to launch, most likely path problem
		perror("Error");
		fprintf(stderr,"Failed to launch: %s\n",runtimeExec.c_str());
		return 1;
	}

	return 0;
}
