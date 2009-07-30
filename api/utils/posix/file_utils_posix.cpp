/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <sys/utsname.h>
#include <libgen.h>
#elif defined(OS_LINUX)
#include <cstdarg>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <libgen.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>

namespace UTILS_NS
{
	std::string FileUtils::GetExecutableDirectory()
	{
#ifdef OS_OSX
		NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
		NSString* contents = [NSString stringWithFormat:@"%@/Contents",bundlePath];
		return std::string([contents UTF8String]);
#elif OS_LINUX
		char tmp[100];
		sprintf(tmp,"/proc/%d/exe",getpid());
		char pbuf[255];
		int c = readlink(tmp,pbuf,255);
		pbuf[c]='\0';
		std::string str(pbuf);
		size_t pos = str.rfind("/");
		if (pos==std::string::npos) return str;
		return str.substr(0,pos);
#endif
	}

	std::string FileUtils::Dirname(std::string path)
	{
		char* pathCopy = strdup(path.c_str());
		std::string toReturn = dirname(pathCopy);
		free(pathCopy);
		return toReturn;
	}

	std::string FileUtils::GetTempDirectory()
	{
#ifdef OS_OSX
		NSString * tempDir = NSTemporaryDirectory();
		if (tempDir == nil)
			tempDir = @"/tmp";

		NSString *tmp = [tempDir stringByAppendingPathComponent:@"kXXXXX"];
		const char * fsTemplate = [tmp fileSystemRepresentation];
		NSMutableData * bufferData =
			[NSMutableData dataWithBytes: fsTemplate length: strlen(fsTemplate)+1];
		char * buffer = (char*)[bufferData mutableBytes];
		mkdtemp(buffer);
		NSString * temporaryDirectory = [[NSFileManager defaultManager]
			stringWithFileSystemRepresentation: buffer
			length: strlen(buffer)];
		return std::string([temporaryDirectory UTF8String]);
#else
		std::ostringstream dir;
		const char* tmp = getenv("TMPDIR");
		const char* tmp2 = getenv("TEMP");
		if (tmp)
			dir << std::string(tmp);
		else if (tmp2)
			dir << std::string(tmp2);
		else
			dir << std::string("/tmp");

		std::string tmp_str = dir.str();
		if (tmp_str.at(tmp_str.length()-1) != '/')
			dir << "/";
		dir << "kXXXXXX";
		char* tempdir = strdup(dir.str().c_str());
		tempdir = mkdtemp(tempdir);
		tmp_str = std::string(tempdir);
		free(tempdir);
		return tmp_str;
#endif
	}

	bool FileUtils::IsFile(std::string &file)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		NSString *f = [NSString stringWithCString:file.c_str()  encoding:NSUTF8StringEncoding];
		NSString *p = [f stringByStandardizingPath];
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:p isDirectory:&isDir];
		return found && !isDir;
#elif OS_LINUX
		struct stat st;
		return (stat(file.c_str(),&st)==0) && S_ISREG(st.st_mode);
#endif
	}

	bool FileUtils::CreateDirectoryImpl(std::string& dir)
	{
#ifdef OS_OSX
		return [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithCString:dir.c_str() encoding:NSUTF8StringEncoding] attributes:nil];
#elif OS_LINUX
		return mkdir(dir.c_str(),0755) == 0;
#endif
	}

	bool FileUtils::DeleteDirectory(std::string &dir)
	{
#ifdef OS_OSX
		[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithCString:dir.c_str() encoding:NSUTF8StringEncoding] handler:nil];
#elif OS_LINUX
		return unlink(dir.c_str()) == 0;
#endif
		return false;
	}

	bool FileUtils::IsDirectory(std::string &dir)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:dir.c_str() encoding:NSUTF8StringEncoding] isDirectory:&isDir];
		return found && isDir;
#elif OS_LINUX
		struct stat st;
		return (stat(dir.c_str(),&st)==0) && S_ISDIR(st.st_mode);
#endif
	}

	std::string FileUtils::Join(const char* inpart, ...)
	{
		va_list ap;
		va_start(ap, inpart);
		std::vector<std::string> parts;
		while (inpart != NULL)
		{
			if (strcmp(inpart, ""))
				parts.push_back(inpart);
			inpart = va_arg(ap, const char*);
		}
		va_end(ap);

		std::string filepath;
		std::vector<std::string>::iterator iter = parts.begin();
		while (iter != parts.end())
		{
			std::string part = *iter;
			bool first = (iter == parts.begin());
			bool last = (iter == parts.end()-1);
			iter++;

			part = Trim(part);
			if (part[part.size()-1] == KR_PATH_SEP_CHAR)
				part = part.erase(part.size() - 1, 1);
			if (!first && part[0] == KR_PATH_SEP_CHAR)
				part = part.erase(0, 1);
			filepath += part;

			if (!last)
				filepath += KR_PATH_SEP;
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithUTF8String:filepath.c_str()] stringByExpandingTildeInPath];
		NSString *p = [s stringByStandardizingPath];
		@try
		{
			filepath = [p fileSystemRepresentation];
		}
		@catch (NSException *ex)
		{
			const char *reason = [[ex reason] UTF8String];
			printf("[Titanium.FileUtils] [Error] Error in Join: %s, '%s'\n", reason, filepath.c_str());
			return filepath;
		}
#endif
		return filepath;
	}

	bool FileUtils::IsHidden(std::string &file)
	{
		// TODO: OS X can also include a 'hidden' flag in file
		// attributes. We should attempt to read this.
		return (file.size() > 0 && file.at(0) == '.');
	}

	std::string FileUtils::GetOSVersion()
	{
#ifdef OS_WIN32
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (GetVersionEx(&vi) == 0) return "?";

		std::ostringstream str;
		str << vi.dwMajorVersion << "." << vi.dwMinorVersion << " (Build " << (vi.dwBuildNumber & 0xFFFF);
		if (vi.szCSDVersion[0]) str << ": " << vi.szCSDVersion;
		str << ")";
		return str.str();
#elif OS_OSX || OS_LINUX
		struct utsname uts;
		uname(&uts);
		return uts.release;
#endif
	}

	std::string FileUtils::GetOSArchitecture()
	{
#ifdef OS_WIN32
		return std::string("win32");
#elif OS_OSX || OS_LINUX
		struct utsname uts;
		uname(&uts);
		return uts.machine;
#endif
	}

	void FileUtils::ListDir(std::string& path, std::vector<std::string> &files)
	{
		if (!IsDirectory(path))
			return;
		files.clear();

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
	}

	int FileUtils::RunAndWait(std::string &path, std::vector<std::string> &args)
	{
		std::string p;
		p+="\"";
		p+=path;
		p+="\" ";
		std::vector<std::string>::iterator i = args.begin();
		while (i!=args.end())
		{
			p+="\"";
			p+=(*i++);
			p+="\" ";
		}

#ifdef DEBUG
		std::cout << "running: " << p << std::endl;
#endif
		int status = system(p.c_str());
		return WEXITSTATUS(status);
	}

	void FileUtils::Unzip(std::string& source, std::string& destination, 
		UnzipCallback callback, void *data)
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
		std::string cmdline = "/usr/bin/ditto";
		RunAndWait(cmdline, args);
#elif OS_LINUX
		std::vector<std::string> args;
		args.push_back("-qq");
		args.push_back("-o");
		args.push_back(source);
		args.push_back("-d");
		args.push_back(destination);
		std::string cmdline("/usr/bin/unzip");
		RunAndWait(cmdline,args);
#endif
	}
}

