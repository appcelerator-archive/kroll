/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>
#include <vector>
#include <api/file_utils.h>

using namespace kroll;

void parseCmdline(std::string &str, std::vector<std::string> &tokens)
{
	std::string::size_type pos = str.find_first_of("\"",0);
	std::string::size_type nextPos = str.find_first_of("\"",pos+1);
	tokens.push_back(str.substr(pos+1,nextPos-pos-1));

	pos = str.find_first_of("\"",nextPos+1);
	nextPos = str.rfind("\"");
	tokens.push_back(str.substr(pos+1,nextPos-pos-1));
}

#ifdef WIN32_CONSOLE
int main (int argc, char **argv)
#else
int WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int)
#endif
{
	std::vector<std::string> argtok;
#ifdef WIN32_CONSOLE
	for (int i = 0; i < argc; i++)
		argtok.push_back(argv[i]);
#else
	std::string argv(cmdline);
	parseCmdline(argv,argtok);
#endif
	if (argtok.size()!=3)
	{
		fprintf(stderr,"Invalid arguments passed (expected 2 args, got %d)\n", argtok.size());
		return __LINE__;
	}
	std::string source = argtok.at(1);
	std::string dest = argtok.at(2);

	if (!FileUtils::IsDirectory(dest))
	{
		CreateDirectory(dest.c_str(),0);
	}

	std::string delimeter("-");

	std::vector<std::string> files;
	FileUtils::ListDir(source,files);
	std::vector<std::string>::iterator iter = files.begin();

	while (iter!=files.end())
	{
		std::string fn = (*iter++);
		if (fn.substr(0,1)=="." || fn.substr(0,2)=="..") continue;
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
				dir.append("\\runtime");
				CreateDirectory(dir.c_str(),0);
				dir.append("\\");
				dir.append(subtype);
				CreateDirectory(dir.c_str(),0);
				dir.append("\\");
				dir.append(version);
				CreateDirectory(dir.c_str(),0);
			}
			else if(type=="module")
			{
				dir.append("\\modules");
				CreateDirectory(dir.c_str(),0);
				dir.append("\\");
				dir.append(subtype);
				CreateDirectory(dir.c_str(),0);
				dir.append("\\");
				dir.append(version);
				CreateDirectory(dir.c_str(),0);
			}
			else
			{
				fprintf(stderr,"unrecognizable type: %s, skipping...\n",type.c_str());
				continue;
			}
			std::string zip(source + "\\" + fn);
			FileUtils::Unzip(zip,dir);
		}
	}
	return 0;
}
