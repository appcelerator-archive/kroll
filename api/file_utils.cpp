/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "file_utils.h"

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#elif defined(OS_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <process.h>
#elif defined(OS_LINUX)
#include <cstdarg>
#include <unistd.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>


namespace kroll
{
	static bool CompareVersions(std::string a, std::string b)
	{
		int a1 = FileUtils::MakeVersion(a);
		int b1 = FileUtils::MakeVersion(b);
		return b1 > a1;
	}
	std::string FileUtils::GetApplicationDirectory()
	{
#ifdef OS_OSX
		NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
		NSString* contents = [NSString stringWithFormat:@"%@/Contents",bundlePath];
		return std::string([contents UTF8String]);
#elif OS_WIN32
		char path[MAX_PATH];
		GetModuleFileName(NULL,path,MAX_PATH);
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
#ifdef OS_WIN32
		char path[MAX_PATH];
		int size = GetEnvironmentVariable("KR_RUNTIME_HOME",(char*)path,MAX_PATH);
		path[size]='\0';
		std::string dir = path;
#else
		std::string dir = getenv("KR_RUNTIME_HOME");
#endif
		dir+=KR_PATH_SEP;
		dir+="appdata";
		if (!IsDirectory(dir))
		{
			CreateDirectory(dir);
		}
		dir+=KR_PATH_SEP;
		dir+=appid;
		if (!IsDirectory(dir))
		{
			CreateDirectory(dir);
		}
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
		TCHAR szTempName[BUFSIZE];
		GetTempPath(BUFSIZE,szTempName);
		std::ostringstream s;
		srand(GetTickCount()); // initialize seed
		std::string dir(szTempName);
		s << dir;
		s << "\\k";
		s << (double)rand();
		return s.str();
#else
		char t[] = "kXXXXXX";
		char* tempdir = mkdtemp(t);
		printf("Tempdir: %s\n", tempdir);

		std::ostringstream dir;
		const char* tmp = getenv("TMPDIR");
		if (tmp)
		{
			dir << std::string(tmp);
		}
		else
		{
			const char *tmp2 = getenv("TEMP");
			if (tmp2)
			{
				dir << std::string(tmp2);
			}
			else
			{
				dir << std::string("/tmp");
			}
		}
		dir << "/k" << (double)rand();
		return dir.str();
#endif
	}
	std::string FileUtils::GetResourcesDirectory()
	{
#ifdef OS_OSX
		NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
		std::string dir = std::string([resourcePath UTF8String]);
#elif OS_WIN32
		std::string dir = FileUtils::GetApplicationDirectory();
		dir.append("\\Resources");
#elif OS_LINUX
		// TODO test this
		std::string dir = FileUtils::GetApplicationDirectory();
		dir.append("/Resources");
#endif
		return dir;
	}
	bool FileUtils::IsFile(std::string &file)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:file.c_str()] isDirectory:&isDir];
		return found && !isDir;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(file.c_str(), &findFileData);
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
	bool FileUtils::CreateDirectory(std::string &dir)
	{
#ifdef OS_OSX
		return [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithCString:dir.c_str()] attributes:nil];
#elif OS_WIN32
		return ::CreateDirectory(dir.c_str(),NULL);
#elif OS_LINUX
		return mkdir(dir.c_str(),0755) == 0;
#endif
		return false;
	}
	bool FileUtils::DeleteDirectory(std::string &dir)
	{
#ifdef OS_OSX
		[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithCString:dir.c_str()] handler:nil];
#elif OS_WIN32
		SHFILEOPSTRUCT op;
		op.hwnd = NULL;
		op.wFunc = FO_DELETE;
		op.pFrom = dir.c_str();
		op.pTo = NULL;
		op.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
		int rc = SHFileOperation(&op);
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
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(dir.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000010) == 0x00000010;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		struct stat st;
		return (stat(dir.c_str(),&st)==0) && S_ISDIR(st.st_mode);
#endif
	}

	const char* FileUtils::GetDirectory(std::string &file)
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
		return file.substr(0,pos).c_str();
#endif
	}

	std::string FileUtils::Join(const char* path, ...)
	{
		va_list ap;
		va_start(ap, path);
		std::vector<std::string> parts;
		parts.push_back(std::string(path));
		while (true)
		{
			const char *i = va_arg(ap,const char*);
			if (i == NULL) break;
			parts.push_back(std::string(i));
		}
		va_end(ap);
		std::string filepath;
		std::vector<std::string>::iterator iter = parts.begin();
		while (iter!=parts.end())
		{
			std::string p = (*iter++);
			filepath+=p;
			if (filepath.length() != 0 && iter!=parts.end())
			{
				filepath+=KR_PATH_SEP;
			}
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithCString:filepath.c_str()] stringByExpandingTildeInPath];
		return std::string([s fileSystemRepresentation]);
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
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(file.c_str(), &findFileData);
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
	bool FileUtils::IsRuntimeInstalled()
	{
		std::string dir = GetRuntimeBaseDirectory();
		return IsDirectory(dir);
	}

	std::string FileUtils::GetRuntimeBaseDirectory()
	{
#ifdef OS_WIN32
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
#elif OS_OSX
		// check to see if we already have a local one
		NSString *localDir = [[NSString stringWithFormat:@"~/Library/Application Support/%s",PRODUCT_NAME] stringByExpandingTildeInPath];
		std::string ls = std::string([localDir UTF8String]);
		if (IsDirectory(ls))
		{
			return std::string([localDir UTF8String]);
		}

		// first check to see if we can install in system directory by checking
		// if we can write to it
		NSString *systemPath = @"/Library/Application Support";
		if ([[NSFileManager defaultManager] isWritableFileAtPath:systemPath])
		{
			return std::string([[systemPath stringByAppendingString:@"/"PRODUCT_NAME] UTF8String]);
		}
		// if not, we fall back to installing into user directory
		return std::string([localDir UTF8String]);
#elif OS_LINUX
		std::string uid = std::string(getenv("USER"));
		std::string p(INSTALL_PREFIX);
		p.append("/");
		p.append(PRODUCT_NAME);
		bool writable = (access(p.c_str(),W_OK)) == 0;
		if (uid.find("root")!=std::string::npos || writable)
		{
			return p;
		}
		passwd *user = getpwnam(uid.c_str());
		char *home = user->pw_dir;
		std::string str(home);
		str.append("/");
		str.append(PRODUCT_NAME);
		return str;
#endif
	}

	void FileUtils::ExtractVersion(std::string& spec, int *op, std::string &version)
	{
		if (spec.find(">=")!=std::string::npos)
		{
			*op = GreaterThanEqualTo;
			version = spec.substr(2,spec.length());
		}
		else if (spec.find("<=")!=std::string::npos)
		{
			*op = LessThanEqualTo;
			version = spec.substr(2,spec.length());
		}
		else if (spec.find("<")!=std::string::npos)
		{
			*op = LessThan;
			version = spec.substr(1,spec.length());
		}
		else if (spec.find(">")!=std::string::npos)
		{
			*op = GreaterThan;
			version = spec.substr(1,spec.length());
		}
		else if (spec.find("=")!=std::string::npos)
		{
			*op = EqualTo;
			version = spec.substr(1,spec.length());
		}
		else
		{
			*op = EqualTo;
			version = spec;
		}
	}
	int FileUtils::MakeVersion(std::string& ver)
	{
		std::string v;
		size_t pos = 0;
		while(1)
		{
			size_t newpos = ver.find(".",pos);
			if (newpos==std::string::npos)
			{
				v.append(ver.substr(pos,ver.length()));
				break;
			}
			v.append(ver.substr(pos,newpos));
			pos = newpos+1;
		}
		return atoi(v.c_str());
	}
	std::string FileUtils::FindVersioned(std::string& path, int op, std::string& version)
	{
		std::vector<std::string> files;
		std::vector<std::string> found;
		ListDir(path,files);
		std::vector<std::string>::iterator iter = files.begin();
		int findVersion = MakeVersion(version);
		while (iter!=files.end())
		{
			std::string str = (*iter++);
			std::string fullpath = std::string(path);
			fullpath.append(KR_PATH_SEP);
			fullpath.append(str);
			if (IsDirectory(fullpath))
			{
				int theVersion = MakeVersion(str);
				bool matched = false;

				switch(op)
				{
					case EqualTo:
					{
						matched = (theVersion == findVersion);
						break;
					}
					case GreaterThanEqualTo:
					{
						matched = (theVersion >= findVersion);
						break;
					}
					case LessThanEqualTo:
					{
						matched = (theVersion <= findVersion);
						break;
					}
					case GreaterThan:
					{
						matched = (theVersion > findVersion);
						break;
					}
					case LessThan:
					{
						matched = (theVersion < findVersion);
						break;
					}
				}
				if (matched)
				{
					found.push_back(std::string(str));
				}
			}
		}
		if (found.size() > 0)
		{
			std::sort(found.begin(),found.end(),CompareVersions);
			std::string file = found.at(0);
			std::string f = std::string(path);
			f.append(KR_PATH_SEP);
			f.append(file);
			return f;
		}
		return std::string();
	}
	std::string FileUtils::GetMachineId()
	{
#ifdef OS_OSX
		kern_return_t kernResult;
		mach_port_t machPort;
		char serialNumber[256];

		kernResult = IOMasterPort( MACH_PORT_NULL, &machPort );

		serialNumber[0] = 0;

		// if we got the master port
		if ( kernResult == KERN_SUCCESS )
		{
			// create a dictionary matching IOPlatformExpertDevice
			CFMutableDictionaryRef classesToMatch = IOServiceMatching("IOPlatformExpertDevice" );

			// if we are successful
			if (classesToMatch)
			{
				// get the matching services iterator
				io_iterator_t iterator;
				kernResult = IOServiceGetMatchingServices( machPort,
				classesToMatch, &iterator );

				// if we succeeded
				if ( (kernResult == KERN_SUCCESS) && iterator )
				{
					io_object_t serviceObj;
					bool done = false;
					do {
						// get the next item out of the dictionary
						serviceObj = IOIteratorNext( iterator );

						// if it is not NULL
						if (serviceObj)
						{
							CFDataRef data = (CFDataRef) IORegistryEntryCreateCFProperty( serviceObj, CFSTR("serial-number"), kCFAllocatorDefault, 0 );

							if (data != NULL)
							{
								CFIndex datalen = CFDataGetLength(data);
								const UInt8* rawdata = CFDataGetBytePtr(data);
								char dataBuffer[256];
								memcpy(dataBuffer, rawdata, datalen);
								sprintf(serialNumber, "%s%s", dataBuffer+13,dataBuffer);
								CFRelease(data);
								done = true;
							}
						}

					} while (done == false);

					IOObjectRelease(serviceObj);
				}

				IOObjectRelease(iterator);
			}
		}
		return std::string(serialNumber);
#elif defined(OS_WIN32)
		//http://www.codeguru.com/cpp/i-n/network/networkinformation/article.php/c5451
		return std::string();	//FIXME: implement
#else
		//http://adywicaksono.wordpress.com/2007/11/08/detecting-mac-address-using-c-application/
		return std::string();	//FIXME: implement
#endif
	}
	void FileUtils::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters)
	{
		std::string::size_type lastPos = str.find_first_not_of(delimeters,0);
		std::string::size_type pos = str.find_first_of(delimeters,lastPos);
		while (std::string::npos!=pos || std::string::npos!=lastPos)
		{
			tokens.push_back(str.substr(lastPos,pos-lastPos));
			lastPos = str.find_first_not_of(delimeters,pos);
			pos = str.find_first_of(delimeters,lastPos);
		}
	}
	std::string FileUtils::Trim(std::string str)
	{
		std::string c(str);
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
	bool FileUtils::ReadManifest(std::string& path, std::string &runtimePath, std::vector< std::pair< std::pair<std::string,std::string>, bool> >& modules, std::vector<std::string> &moduleDirs, std::string &appname, std::string &appid, std::string &runtimeOverride)
	{
		std::ifstream file(path.c_str());
		if (file.bad() || file.fail())
		{
			return false;
		}
		bool foundRuntime = false;
		const char *rt = runtimeOverride.c_str();
#ifdef DEBUG
				std::cout << "Read Manifest: " << rt << std::endl;
#endif

		while (!file.eof())
		{
			std::string line;
			std::getline(file,line);
			if (line.find(" ")==0)
			{
				continue;
			}
			size_t pos = line.find(":");
			if (pos!=std::string::npos)
			{
				std::string key = Trim(line.substr(0,pos));
				std::string value = Trim(line.substr(pos+1,line.length()));
				if (key == "#appname")
				{
					appname = value;
					continue;
				}
				else if (key == "#appid")
				{
					appid = value;
					continue;
				}
				else if (key.c_str()[0]=='#')
				{
					continue;
				}
				int op;
				std::string version;
				ExtractVersion(value,&op,version);
#ifdef DEBUG
				std::cout << "Component: " << key << ":" << version << ", operation: " << op << std::endl;
#endif
				std::pair<std::string,std::string> p(key,version);
				if (key == "runtime")
				{
					// check to see if our runtime is found in our override directory
					if (!runtimeOverride.empty())
					{
						std::string potentialRuntime = Join(rt,"runtime",NULL);
						if (IsDirectory(potentialRuntime))
						{
							// extra special check for Win32 since we have to place the WebKit.dll
							// inside the same relative path as .exe because of the COM embedded manifest crap-o-la
							// so if we can't find kroll.dll in the resources folder we don't override
#ifdef OS_WIN32
							std::string krolldll = kroll::FileUtils::Join(potentialRuntime.c_str(),"kroll.dll",NULL);
							if (kroll::FileUtils::IsFile(krolldll))
							{
#endif						
								runtimePath = potentialRuntime;
#ifdef DEBUG
								std::cout << "found override runtime at: " << runtimePath << std::endl;
#endif
								foundRuntime = true;
								continue;
#ifdef OS_WIN32
							}
#endif						
						}
					}
					runtimePath = FindRuntime(op,version);
					if (runtimePath == "")
					{
						modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,false));
					}
					else
					{
						foundRuntime = true;
					}
				}
				else
				{
					// check to see if our module is contained within our runtime override
					// directory and if it is, use it...
					std::string potentialModule = kroll::FileUtils::Join(rt,"modules",key.c_str(),NULL);

#ifdef DEBUG
					std::cout << "looking for bundled module at " << potentialModule << std::endl;
#endif
					if (IsDirectory(potentialModule))
					{
						modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,true));
						moduleDirs.push_back(potentialModule);
#ifdef DEBUG
						std::cout << "found override module at: " << potentialModule << std::endl;
#endif
						continue;
					}
					std::string dir = FindModule(key,op,version);
					bool found = dir!="";
					modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,found));
					if (found)
					{
#ifdef DEBUG
						std::cout << "found module at: " << dir << std::endl;
#endif
						moduleDirs.push_back(dir);
					}
#ifdef DEBUG
					else
					{
						std::cerr << "couldn't find module module: " << key  << "/" << version << std::endl;
					}
#endif
				}
			}
		}
		// we gotta always have a runtime
		if (!foundRuntime)
		{
			std::pair<std::string,std::string> p("runtime","0.2"); //TODO: huh, what do we use?
			modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,false));
		}
		file.close();
		return true;
	}
	std::string FileUtils::FindRuntime(int op, std::string& version)
	{
		std::string runtime = GetRuntimeBaseDirectory();
		std::string path(runtime);
#ifdef OS_WIN32
		path += "\\runtime\\win32";
#elif OS_LINUX
		path += "/runtime/linux";
#elif OS_OSX
		path += "/runtime/osx";
#endif
		return FindVersioned(path,op,version);
	}
	std::string FileUtils::FindModule(std::string& name, int op, std::string& version)
	{
		std::string runtime = GetRuntimeBaseDirectory();
		std::string path(runtime);
#ifdef OS_WIN32
		path += "\\modules\\";
#else
		path += "/modules/";
#endif
		path += name;
		return FindVersioned(path,op,version);
	}
	void FileUtils::ListDir(std::string& path, std::vector<std::string> &files)
	{
	#if defined(OS_WIN32)

		WIN32_FIND_DATA findFileData;
		std::string q(path+"\\*");
		HANDLE hFind = FindFirstFile(q.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				files.push_back(std::string(findFileData.cFileName));
			} while (FindNextFile(hFind, &findFileData));
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
		return system(p.c_str());
#elif defined(OS_WIN32)
		std::ostringstream ostr;
		ostr << path.c_str();
		if (args.size() > 0 )
		{
			std::vector<std::string>::iterator i = args.begin();
			int idx = 0;
			while (i!=args.end())
			{
				// we need to quote each argument
				ostr << " \"" << (*i++).c_str() << "\"";
			}
		}
		DWORD rc=0;
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		char buf[MAX_PATH];
		DWORD size = GetCurrentDirectory(MAX_PATH,(char*)buf);
		buf[size]='\0';
		if (!CreateProcess( NULL,   // No module name (use command line)
							(char*)ostr.str().c_str(), // Command line
							NULL,           // Process handle not inheritable
							NULL,           // Thread handle not inheritable
							FALSE,          // Set handle inheritance to FALSE
							0,              // No creation flags
							NULL,           // Use parent's environment block
							(char*)buf,		// Use parent's starting directory 
							&si,            // Pointer to STARTUPINFO structure
							&pi )           // Pointer to PROCESS_INFORMATION structure
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
	const char* FileUtils::GetUsername()
	{
#ifdef OS_OSX
		return [NSUserName() UTF8String];
#elif OS_WIN32
		char buf[MAX_PATH];
		DWORD size = MAX_PATH;
        if (::GetUserName(buf,&size))
		{
			buf[size]='\0';
		}
		else
		{
			sprintf(buf,"Unknown");
		}
		return std::string(buf).c_str();
#elif OS_LINUX
		return getlogin();
#endif
	}
#ifndef NO_UNZIP
	void FileUtils::Unzip(std::string& source, std::string& destination)
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
		for (int zi=0; zi < numitems; zi++)
		{
			GetZipItem(hz,zi,&ze);
			UnzipItem(hz,zi,ze.name);
		}
		CloseZip(hz);
#endif
	}
#endif
}

