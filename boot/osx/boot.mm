/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <string>
#import <api/file_utils.h>

using namespace kroll;

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
- (BOOL)launch:(NSInvocation*) terminated{
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
	return YES;
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

int main(int argc, char* argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
	NSString* bundleName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
	
	// 1. read the application manifest to determine what's needed
	NSString *manifestNS = [NSString stringWithFormat:@"%@/manifest",bundlePath];

	std::string ms = std::string([manifestNS UTF8String]);
	if (!FileUtils::IsFile(ms))
	{
		NSLog(@"invalid bundle. manifest file doesn't exist at %@",manifestNS);
		[pool release];
		return __LINE__;
	}

	std::vector< std::pair< std::pair<std::string,std::string>,bool> > modules;
	std::vector<std::string> moduleDirs;
	std::string manifest([manifestNS UTF8String]);
	std::string runtimePath;
	bool success = FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs);
	if (!success)
	{
		NSLog(@"Could not read manifest: %@", manifestNS);
		[pool release];
		return __LINE__;
	}
	
	std::vector< std::pair<std::string,std::string> > missing;
	std::vector< std::pair< std::pair<std::string,std::string>, bool> >::iterator i = modules.begin();
	while(i!=modules.end())
	{
		std::pair< std::pair<std::string,std::string>,bool> p = (*i++);
		if (!p.second)
		{
			missing.push_back(p.first);
			NSLog(@"missing module: %s/%s",p.first.first.c_str(),p.first.second.c_str());
		}
	}
	std::string runtimeBase = FileUtils::GetRuntimeBaseDirectory();

	if (missing.size()>0)
	{
		const char *updatesite = getenv("TITANIUM_UPDATESITE");
		std::string url;
		if (!updatesite)
		{
			//FIXME
//			url = "http://updatesite.titaniumapp.com/";

			url = "http://localhost/~";
			url+=[NSUserName() UTF8String];
			url+="/titanium";
			NSLog(@"path => %s",url.c_str());
		}
		else
		{
			url = std::string(updatesite);
		}
		//TODO: delete this directory!
		std::string sourceTemp = FileUtils::GetTempDirectory();
		
		std::vector<std::string> args;
		// appname
		args.push_back([bundleName UTF8String]);
		// title 
		//I18N: localize these
		args.push_back("Additional application files required");
		// message
		//I18N: localize these
		args.push_back([[NSString stringWithFormat:@"There are additional application files that are required for %@. These will be downloaded from the network. Please press Continue to download these files now to complete the installation of %@.",bundleName,bundleName] UTF8String]);
		// extract directory
		args.push_back(sourceTemp);
		// runtime base
		args.push_back(runtimeBase);

		// make sure we create our runtime directory
		[[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithCString:runtimeBase.c_str()] attributes:nil];
		
		//TODO: get aid
		std::string aid;
		std::string sid = kroll::FileUtils::GetMachineId();
		std::string qs("?os=osx&sid="+sid+"&aid="+aid);
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
				NSString *rt = [NSString stringWithFormat:@"%@/installer/runtime",bundlePath];
				path = std::string([rt UTF8String]);
				if (FileUtils::IsDirectory(path))
				{
					found = true;
					runtimePath = path;
				}
			}
			else
			{
				name = "module-" + p.first + "-" + p.second;
				// see if we have a private module installed and we can link to that
				NSString *pm = [NSString stringWithFormat:@"%@/installer/modules/%s",bundlePath,p.first.c_str()];
				path = std::string([pm UTF8String]);
				if (FileUtils::IsDirectory(path))
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
				NSURL *u = [NSURL URLWithString:[NSString stringWithFormat:@"%s/%s.zip%s",url.c_str(),name.c_str(),qs.c_str()]];
				args.push_back([[u absoluteString] UTF8String]);
				missingCount++;
			}
		}

		// we have to check again in case the private module/runtime was
		// resolved inside the application folder
		if (missingCount>0)
		{
			// run the installer app which will fetch remote modules/runtime for us
			NSString *exec = [NSString stringWithFormat:@"%@/installer/Installer App.app/Contents/MacOS/Installer App",bundlePath];
			NSLog(@"launching: %@",exec);
		
			FileUtils::RunAndWait([exec UTF8String],args);
		
			//TODO: improve check for error/cancel
			modules.clear();
			moduleDirs.clear();
			bool success = FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs);
			if (!success || modules.size()!=moduleDirs.size())
			{
				// must have failed
				// unlink the temporary directory
				[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithCString:sourceTemp.c_str()] handler:nil];
				[pool release];
				return __LINE__;
			}

			NSLog(@"new runtime path is: %s",runtimePath.c_str());
		}
		// unlink the temporary directory
		[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithCString:sourceTemp.c_str()] handler:nil];
	}

	// 2. now we're going to make sure we have our application structure setup
	NSString *appRootDir = [NSString stringWithFormat:@"%@/app",bundlePath];
	NSString *appDir = [NSString stringWithFormat:@"%@/%@.app",appRootDir,bundleName];
	NSString *appContentsDir = [NSString stringWithFormat:@"%@/Contents",appDir];
	NSString *appResourcesDir = [NSString stringWithFormat:@"%@/Contents/Resources",appDir];
	NSString *appFrameworksDir = [NSString stringWithFormat:@"%@/Contents/Frameworks", appDir];
	NSString *appMacOSDir = [NSString stringWithFormat:@"%@/Contents/MacOS",appDir];
	
	NSString *runtimeExec = [NSString stringWithFormat:@"%@/%@",appMacOSDir,bundleName];
	
	// we need to construct a parallel structure for the application
	// to trick OSX into thinking that the tikernel process is our 
	// real application bundle - which really is just a set of symlinks
	std::string ads = std::string([appDir UTF8String]);
	if (!FileUtils::IsDirectory(ads))
	{
		NSFileManager *fm = [NSFileManager defaultManager];
		[fm createDirectoryAtPath:appRootDir attributes:nil];
		[fm createDirectoryAtPath:appDir attributes:nil];
		[fm createDirectoryAtPath:appContentsDir attributes:nil];
		[fm createDirectoryAtPath:appMacOSDir attributes:nil];
		
		// copy our Resources into our Resources folder
		NSString *srcResources = [NSString stringWithFormat:@"%s/Resources/English.lproj",runtimePath.c_str()];
		NSString *destResources = [NSString stringWithFormat:@"%@/Contents/Resources/English.lproj",bundlePath];
		
		[fm createSymbolicLinkAtPath:destResources pathContent:srcResources];
	
		NSString *tiKernel = [NSString stringWithFormat:@"%s/kkernel",runtimePath.c_str()];		
		[fm createSymbolicLinkAtPath:runtimeExec pathContent:tiKernel];
		
		NSString *srcInfoPlist = [NSString stringWithFormat:@"%@/Contents/Info.plist",bundlePath];		
		NSString *infoPlist = [NSString stringWithFormat:@"%@/Contents/Info.plist",appDir];		
		[fm createSymbolicLinkAtPath:infoPlist pathContent:srcInfoPlist];
	
		NSString *srcXML = [NSString stringWithFormat:@"%@/Contents/%s",bundlePath,CONFIG_FILENAME];		
		NSString *destXML = [NSString stringWithFormat:@"%@/Contents/%s",appDir,CONFIG_FILENAME];		
		[fm createSymbolicLinkAtPath:destXML pathContent:srcXML];
	
		NSString *srcResourcesDir = [NSString stringWithFormat:@"%@/Contents/Resources",bundlePath];
		[fm createSymbolicLinkAtPath:appResourcesDir pathContent:srcResourcesDir];
		
		NSString *srcFrameworksDir = [NSString stringWithFormat:@"%@/Contents/Frameworks", bundlePath];
		std::string fdir([srcFrameworksDir UTF8String]);
		if (FileUtils::IsDirectory(fdir)) {
			[fm createSymbolicLinkAtPath:appFrameworksDir pathContent:srcFrameworksDir];
		}
	}
	
	NSMutableString *libpath = [NSMutableString stringWithCapacity:255];
	[libpath appendString:[NSString stringWithCString:runtimePath.c_str()]];
	[libpath appendString:@":"];

	NSMutableString *modulePath = [NSMutableString stringWithCapacity:255];

	for (int c=0;c<(int)moduleDirs.size();c++)
	{
		std::string module = moduleDirs.at(c);
		NSString *p = [NSString stringWithCString:module.c_str()];
		[libpath appendString:p];
		[libpath appendString:@":"];
		[modulePath appendString:p];
		[modulePath appendString:@":"];
	}
	
	NSString *startDir = appContentsDir;

	// create our program args (just pass what was passed to us)
	NSMutableArray *args = [[[NSMutableArray alloc] init] autorelease];
	for (int c=1;c<argc;c++)
	{
		[args addObject:[NSString stringWithFormat:@"%s",argv[c]]];
		std::string arg(argv[c]);
		int t = arg.find("--kstart=");
		if (t==0)
		{
			std::string value = arg.substr(arg.find("=")+1);
			// this means start the app from a different home directory
			// we use kstart to (hopefully) not conflict with a normal
			// Kroll app's own command line variables
			startDir = [NSString stringWithCString:value.c_str()];
		}
	}
	
	NSLog(@"start directory is = %@",startDir);
	
	//TODO: we need to refactor this out since these are Titanium specific
	//for now, it doesn't hurt if you don't have them
	NSString *dylibPath = [NSString stringWithFormat:@"%@:%s/WebKit.framework/Versions/Current",libpath,runtimePath.c_str()];
	dylibPath = [dylibPath stringByAppendingFormat:@":%s/WebCore.framework/Versions/Current",runtimePath.c_str()];
	dylibPath = [dylibPath stringByAppendingFormat:@":%s/JavaScriptCore.framework/Versions/Current",runtimePath.c_str()];
	dylibPath = [dylibPath stringByAppendingFormat:@":%s",runtimePath.c_str()];

	NSString *runtime = [NSString stringWithCString:runtimeBase.c_str()];
	// create our environment
	NSDictionary *ce = [[NSProcessInfo processInfo] environment];
	NSMutableDictionary *env = [[NSMutableDictionary alloc] initWithDictionary:ce];
	[env setValue:startDir forKey:@"KR_HOME"];
	[env setValue:[NSString stringWithCString:runtimePath.c_str()] forKey:@"KR_RUNTIME"];
	[env setValue:runtime forKey:@"KR_RUNTIME_HOME"];
	[env setValue:modulePath forKey:@"KR_PLUGINS"];
	[env setValue:dylibPath forKey:@"DYLD_LIBRARY_PATH"];
	
	NSLog(@"executing: %@ %@ with %@, path=%@",runtimeExec,args,env,bundlePath);

	// 4. now launch the process. 
	invoker = [TaskInvoker alloc];
	NSInvocation* terminateInvocation = [TaskCallback createInvocation: @selector(terminated:)];

	// setup termination handlers that will ensure that we don't 
	// orphan our subprocess
	signal(SIGHUP, &termination);
	signal(SIGTERM, &termination);
	signal(SIGINT, &termination);
	signal(SIGKILL, &termination);

	[invoker initWithApplication:runtimeExec environment:env arguments:args currentDirectory:bundlePath];
	[invoker launch:terminateInvocation];
	[pool release];
	
	return 0;
}
