/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <api/file_utils.h>

using namespace kroll;

int main(int argc, char* argv[])
{
	if (argc!=3)
	{
		fprintf(stderr,"Invalid arguments passed\n");
		return __LINE__;
	}	
	char *source = argv[1];
	std::string dest = argv[2];

	if (!FileUtils::IsDirectory(dest))
	{
		mkdir(dest.c_str(),0755);
	}
		
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(source)) == NULL)
	{
		fprintf(stderr,"Couldn't list contents of: %s\n",source);
		return __LINE__;
	}

	std::string delimeter("-");

	while ((dirp=readdir(dp))!=NULL)
	{
		std::string fn = std::string(dirp->d_name);
		if (fn.substr(0,1)=="." || fn.substr(0,2)=="..")
		{
			continue;
		}
		if (fn.find(".zip")!=std::string::npos)
		{
			std::vector<std::string> tokens;
			FileUtils::Tokenize(fn,tokens,delimeter);
			// paranoia check
			if (tokens.size()!=3)
			{
				fprintf(stderr,"Invalid zip format: %s, skipping...\n",fn.c_str());
				continue;
			}
			std::string type = tokens.at(0);
			std::string subtype = tokens.at(1);
			std::string version = tokens.at(2);	
			size_t pos = version.rfind(".");
   			version = version.substr(0,pos);

			std::string dir(dest);
			if (type=="runtime")
			{
				dir.append("/runtime");
				mkdir(dir.c_str(),0755);
				dir.append("/");
				dir.append(subtype);
				mkdir(dir.c_str(),0755);
				dir.append("/");
				dir.append(version);
				mkdir(dir.c_str(),0755);
			}
			else if(type=="module")
			{
				dir.append("/modules");
				mkdir(dir.c_str(),0755);
				dir.append("/");
				dir.append(subtype);
				mkdir(dir.c_str(),0755);
				dir.append("/");
				dir.append(version);
				mkdir(dir.c_str(),0755);
			}
			else
			{
				fprintf(stderr,"unrecognizable type: %s, skipping...\n",type.c_str());
				continue;
			}
			std::string cmdline(source);
			cmdline.append("/");
			cmdline.append(fn);
			FileUtils::Unzip(cmdline,dir);
		}	
	}

	closedir(dp);
	
	return 0;
}
