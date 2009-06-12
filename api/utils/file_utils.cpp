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
#elif defined(OS_WIN32)
#include "../base.h"
#include <windows.h>
#include <shlobj.h>
#include <Iphlpapi.h>
#include <process.h>
#include <shellapi.h>
#elif defined(OS_LINUX)
#include <cstdarg>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sys/utsname.h>
#include <libgen.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>

const std::string ILLEGAL = "<>{}|\\\"^`";

static std::string safe_encode(std::string &str)
{
	std::string encodedStr;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		char c = *it;
		if (isalnum(c) ||
			c == '-' || c == '_' ||
			c == '.' || c == '~' ||
			c == '/' || c == '\\' ||
			c == ' ')
		{
			encodedStr += c;
		}
		else if (c == ' ')
		{
			encodedStr += c;
		}
		else if (c <= 0x20 || c >= 0x7F || ILLEGAL.find(c) != std::string::npos)
		{
			// skip these bad out of range characters ....
		}
		else encodedStr += c;
	}
	return encodedStr;
}


namespace UTILS_NS
{
	std::string FileUtils::GetExecutableDirectory()
	{
#ifdef OS_OSX
		NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
		NSString* contents = [NSString stringWithFormat:@"%@/Contents",bundlePath];
		return std::string([contents UTF8String]);
#elif OS_WIN32
		char path[MAX_PATH];
		GetModuleFileNameA(NULL,path,MAX_PATH);
		std::string p(path);
		std::string::size_type pos = p.rfind("\\");
		if (pos!=std::string::npos)
		{
		  return p.substr(0,pos);
		}
	  	return p;
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

	std::string FileUtils::GetApplicationDataDirectory(std::string &appid)
	{
		std::string dir = GetUserRuntimeHomeDirectory();
		dir = FileUtils::Join(dir.c_str(), "appdata", appid.c_str(), NULL);
		CreateDirectory(dir, true);

		return dir;
	}

	std::string FileUtils::GetTempDirectory()
	{
#ifdef OS_OSX
		NSString * tempDir = NSTemporaryDirectory();
		if (tempDir == nil)
			tempDir = @"/tmp";

		NSString *tmp = [tempDir stringByAppendingPathComponent:@"kXXXXX"];
		const char * fsTemplate = [tmp fileSystemRepresentation];
		NSMutableData * bufferData = [NSMutableData dataWithBytes: fsTemplate
		                                                   length: strlen(fsTemplate)+1];
		char * buffer = (char*)[bufferData mutableBytes];
		mkdtemp(buffer);
		NSString * temporaryDirectory = [[NSFileManager defaultManager]
		        stringWithFileSystemRepresentation: buffer
		                                    length: strlen(buffer)];
		return std::string([temporaryDirectory UTF8String]);
#elif defined(OS_WIN32)
#define BUFSIZE 512
		char szTempName[BUFSIZE];
		GetTempPathA(BUFSIZE, szTempName);
		std::string dir(szTempName);
		srand(GetTickCount()); // initialize seed
		std::ostringstream s;
		s << "k" << (double)rand();
		std::string end = s.str();
		return FileUtils::Join(dir.c_str(), end.c_str(), NULL);
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
		NSString *f = [NSString stringWithCString:file.c_str()];
		NSString *p = [f stringByStandardizingPath];
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:p isDirectory:&isDir];
		return found && !isDir;
#elif OS_WIN32
		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(file.c_str(), &findFileData);
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

	std::string FileUtils::Dirname(std::string path)
	{
#ifdef OS_WIN32
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	strncpy(path_buffer, path.c_str(), _MAX_PATH);
	_splitpath(path_buffer, drive, dir, fname, ext );
	
	if (dir[strlen(dir)-1] == '\\')
		dir[strlen(dir)-1] = '\0';
	std::string dirname = drive;
	dirname += std::string(dir);
	return dirname;
#else
	char* pathCopy = strdup(path.c_str());
	std::string toReturn = dirname(pathCopy);
	free(pathCopy);
	return toReturn;
#endif
	}

	std::string FileUtils::Basename(std::string path)
	{
		size_t pos = path.find_last_of(KR_PATH_SEP_CHAR);
		if (pos == std::string::npos)
			return path;
		else
			return path.substr(pos+1);
	}

	bool FileUtils::CreateDirectory(std::string &dir, bool recursive)
	{
		if (IsDirectory(dir)) {
			return true;
		}
		
		string parent = Dirname(dir);
		if (recursive && parent.size() > 0 && !IsDirectory(parent))
		{
			CreateDirectory(parent, true);
		}
#ifdef OS_OSX
		return [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithCString:dir.c_str()] attributes:nil];
#elif OS_WIN32
		return ::CreateDirectoryA(dir.c_str(),NULL);
#elif OS_LINUX
		return mkdir(dir.c_str(),0755) == 0;
#endif
		return false;
	}

	bool FileUtils::CreateDirectory2(std::string &dir)
	{
		return FileUtils::CreateDirectory(dir);
	}

	bool FileUtils::DeleteDirectory(std::string &dir)
	{
#ifdef OS_OSX
		[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithCString:dir.c_str()] handler:nil];
#elif OS_WIN32
		SHFILEOPSTRUCTA op;
		op.hwnd = NULL;
		op.wFunc = FO_DELETE;
		op.pFrom = dir.c_str();
		op.pTo = NULL;
		op.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
		int rc = SHFileOperationA(&op);
		return (rc == 0);
#elif OS_LINUX
		return unlink(dir.c_str()) == 0;
#endif
		return false;
	}

	bool FileUtils::IsDirectory(std::string &dir)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:dir.c_str()] isDirectory:&isDir];
		return found && isDir;
#elif OS_WIN32
		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(dir.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		else
		{
			if ((findFileData.dwFileAttributes & 0x00000010) == 0x00000010)
			{
				FindClose(hFind);
				return true;
			}
			else
			{
				FindClose(hFind);
				return false;
			}
		}

#elif OS_LINUX
		struct stat st;
		return (stat(dir.c_str(),&st)==0) && S_ISDIR(st.st_mode);
#endif
	}

	std::string FileUtils::GetDirectory(std::string &file)
	{
		size_t pos = file.find_last_of(KR_PATH_SEP);
		if (pos == std::string::npos)
		{
			pos = file.find_last_of(KR_PATH_SEP_OTHER);
			if (pos == std::string::npos)
			{
				return "."KR_PATH_SEP; //??
			}
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithCString:file.substr(0,pos).c_str()] stringByExpandingTildeInPath];
		return [s fileSystemRepresentation];
#else
		return file.substr(0, pos);
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
		NSString *s = [[NSString stringWithCString:filepath.c_str()] stringByExpandingTildeInPath];
		NSString *p = [s stringByStandardizingPath];
		return std::string([p fileSystemRepresentation]);
#else
		return filepath;
#endif
	}

	bool FileUtils::IsHidden(std::string &file)
	{
#ifdef OS_OSX
		// TODO finish this
		return false;
#elif OS_WIN32
		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(file.c_str(), &findFileData);
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

	void FileUtils::Tokenize(
		const std::string& str,
		std::vector<std::string>& tokens, 
		const std::string delimeters, 
		bool skip_if_found)
	{
		std::string::size_type lastPos = str.find_first_not_of(delimeters,0);
		std::string::size_type pos = str.find_first_of(delimeters,lastPos);
		while (std::string::npos!=pos || std::string::npos!=lastPos)
		{
			std::string token = str.substr(lastPos,pos-lastPos);
			bool found = false;
			if (skip_if_found)
			{
				std::vector<std::string>::iterator i = tokens.begin();
				while(i!=tokens.end())
				{
					std::string entry = (*i++);
					if (entry == token)
					{
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				tokens.push_back(token);
			}
			lastPos = str.find_first_not_of(delimeters,pos);
			pos = str.find_first_of(delimeters,lastPos);
		}
	}

	std::string FileUtils::Trim(std::string str)
	{
		std::string c(safe_encode(str));
		while (1)
		{
			size_t pos = c.rfind(" ");
			if (pos == std::string::npos || pos!=c.length()-1)
			{
				break;
			}
			c = c.substr(0,pos);
		}
		while(1)
		{
			size_t pos = c.find(" ");
			if (pos != 0)
			{
				break;
			}
			c = c.substr(1);
		}
		return c;
	}

	void FileUtils::ListDir(std::string& path, std::vector<std::string> &files)
	{
		if (!IsDirectory(path))
			return;
		files.clear();

	#if defined(OS_WIN32)
		WIN32_FIND_DATAA findFileData;
		std::string q(path+"\\*");
		HANDLE hFind = FindFirstFileA(q.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::string fn = std::string(findFileData.cFileName);
				if (fn.substr(0,1) == "." || fn.substr(0,2) == "..") {
					continue;
				}
				files.push_back(fn);
			} while (FindNextFileA(hFind, &findFileData));
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

	int FileUtils::RunAndWait(std::string &path, std::vector<std::string> &args)
	{
#ifndef OS_WIN32
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
#elif defined(OS_WIN32)
		std::string cmdLine = "\"" + path + "\"";
		for (int i = 0; i < args.size(); i++)
		{
			cmdLine += " \"" + args.at(i) + "\"";
		}
		printf("cmd: %s\n", cmdLine.c_str());

		DWORD rc=0;
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		char cwd[MAX_PATH];
		DWORD size = GetCurrentDirectoryA(MAX_PATH, (char*)cwd);
		if (!CreateProcessA(
			NULL,                    // No module name (use command line)
			(char*) cmdLine.c_str(), // Command line
			NULL,                    // Process handle not inheritable
			NULL,                    // Thread handle not inheritable
			FALSE,                   // Set handle inheritance to FALSE
			0,                       // No creation flags
			NULL,                    // Use parent's environment block
			(char*)cwd,		         // Use parent's starting directory
			&si,                     // Pointer to STARTUPINFO structure
			&pi )                    // Pointer to PROCESS_INFORMATION structure
		)
		{
			rc = -1;
		}
		else
		{
			// Wait until child process exits.
			WaitForSingleObject( pi.hProcess, INFINITE );

			// set the exit code
			GetExitCodeProcess(pi.hProcess,&rc);

			// Close process and thread handles.
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}

		return rc;
#endif
	}

#ifndef NO_UNZIP
	void FileUtils::Unzip(std::string& source, std::string& destination, UnzipCallback callback, void *data)
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
		RunAndWait(cmdline,args);
#elif OS_LINUX
		std::vector<std::string> args;
		args.push_back("-qq");
		args.push_back("-o");
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
		char message[1024];
			
		if (callback != NULL) {
			sprintf(message, "Starting extraction of %d items from %s to %s", numitems, source.c_str(), destination.c_str());
			
			callback(message, 0, numitems, data);
		}
		
		for (int zi=0; zi < numitems; zi++)
		{
			GetZipItem(hz,zi,&ze);
			
			if (callback != NULL) {
				sprintf(message, "Extracting %s...", ze.name);
				callback(message, zi, numitems, data);
			}
			
			UnzipItem(hz,zi,ze.name);
		}
		CloseZip(hz);
#endif
	}
#endif
}

