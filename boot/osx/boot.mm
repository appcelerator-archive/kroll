/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <string>
#import <api/kroll.h>

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
	
	BOOL installOnly = NO;
	
	if (argc > 1)
	{
		for (int c=0;c<argc;c++)
		{
			if (strstr(argv[c],"--install")==0)
			{
				installOnly = YES;
				break;
			}
		}
	}

	NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
	NSString* bundleName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
	NSString* runtime = [NSString stringWithCString:FileUtils::GetRuntimeBaseDirectory().c_str()];
	
	NSLog(@"Runtime: %@",runtime);
	NSLog(@"bundleName: %@",bundleName);
	NSLog(@"bundlePath: %@",bundlePath);

	if (!FileUtils::IsRuntimeInstalled())
	{
		// we don't have runtime, we need to launch our installer 
		// to install it which should be inside our bundle directory
		NSString *bundledInstaller = [NSString stringWithFormat:@"%@/installer",bundlePath];
		std::string bdir = std::string([bundledInstaller UTF8String]);
		if (!FileUtils::IsDirectory(bdir))
		{
			NSLog(@"invalid bundle. installer path doesn't exist at %@",bundledInstaller);
			[pool release];
			return __LINE__;
		}
		NSString *installer = [NSString stringWithFormat:@"%@/kinstall", bundledInstaller];
		std::string si = std::string([installer UTF8String]);
		if (!FileUtils::IsFile(si))
		{
			NSLog(@"invalid bundle. installer file doesn't exist at %@",installer);
			[pool release];
			return __LINE__;
		}
		NSLog(@"runtime set to %@",runtime);
		std::vector<std::string> args;
		args.push_back(std::string([bundledInstaller UTF8String]));
		args.push_back(std::string([runtime UTF8String]));
		int exitCode = FileUtils::RunAndWait(std::string([installer UTF8String]),args);
		NSLog(@"exit code %d",exitCode);
	}
	
	// at this point, we now need to determine which runtime and modules
	// so we can launch our runtime kernels with the appropriate paths
	// and environment pre-setup
	
	// 1. read the application manifest to determine what's needed
	NSString *manifestNS = [NSString stringWithFormat:@"%@/manifest",bundlePath];
	std::string ms = std::string([manifestNS UTF8String]);
	if (!FileUtils::IsFile(ms))
	{
		NSLog(@"invalid bundle. manifest file doesn't exist at %@",manifestNS);
		[pool release];
		return __LINE__;
	}

	std::vector<std::string> modules;
	std::vector<std::string> moduleDirs;
	std::string manifest([manifestNS UTF8String]);
	std::string runtimePath;
	FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs);
	bool success = FileUtils::ReadManifest(c,runtimePath,modules,moduleDirs);
	if (!success)
	{
		NSLog(@"Could not read manifest: %@", c);
		[pool release];
		return __LINE__;
	}

	// 2. now we're going to make sure we have our application structure setup
	NSString *appRootDir = [NSString stringWithFormat:@"%@/app",bundlePath];
	NSString *appDir = [NSString stringWithFormat:@"%@/%@.app",appRootDir,bundleName];
	NSString *appContentsDir = [NSString stringWithFormat:@"%@/Contents",appDir];
	NSString *appResourcesDir = [NSString stringWithFormat:@"%@/Contents/Resources",appDir];
	NSString *appMacOSDir = [NSString stringWithFormat:@"%@/Contents/MacOS",appDir];
	
	NSString *runtimeExec = [NSString stringWithFormat:@"%@/%@",appMacOSDir,bundleName];
	NSLog(@"tikernel path = %@",runtimeExec);
	
	// we need to critic a parallel structure for the application
	// to trick OSX into thinking that the tikernel process is our 
	// real application bundle - which really is just a set of symlinks
	std::string ads = std::string([appDir UTF8String]);
	if (!FileUtils::IsDirectory(ads) || installOnly)
	{
		NSFileManager *fm = [NSFileManager defaultManager];
		[fm createDirectoryAtPath:appRootDir attributes:nil];
		[fm createDirectoryAtPath:appDir attributes:nil];
		[fm createDirectoryAtPath:appContentsDir attributes:nil];
		[fm createDirectoryAtPath:appMacOSDir attributes:nil];
		
		// copy our Resources into our Resources folder
		NSString *srcResources = [NSString stringWithFormat:@"%s/Resources",runtimePath.c_str()];
		[fm copyPath:srcResources toPath:appResourcesDir handler:nil];
	
		NSString *tiKernel = [NSString stringWithFormat:@"%s/kkernel",runtimePath.c_str()];		
		[fm createSymbolicLinkAtPath:runtimeExec pathContent:tiKernel];
		
		NSString *srcInfoPlist = [NSString stringWithFormat:@"%@/Contents/Info.plist",bundlePath];		
		NSString *infoPlist = [NSString stringWithFormat:@"%@/Contents/Info.plist",appDir];		
		[fm createSymbolicLinkAtPath:infoPlist pathContent:srcInfoPlist];
	
//FIXME: resolve this with titanium	
		NSString *srctiXML = [NSString stringWithFormat:@"%@/Contents/tiapp.xml",bundlePath];		
		NSString *tiXML = [NSString stringWithFormat:@"%@/Contents/tiapp.xml",appDir];		
		[fm createSymbolicLinkAtPath:tiXML pathContent:srctiXML];
	
		NSString *srcResourcesDir = [NSString stringWithFormat:@"%@/Contents/Resources",bundlePath];
		[fm createSymbolicLinkAtPath:appResourcesDir pathContent:srcResourcesDir];
	}
	
	// if we passed in --install, we're going to just exit after we've
	// done the install setup for the application
	if (installOnly == NO)
	{
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
	
		// create our program args (just pass what was passed to us)
		NSMutableArray *args = [[[NSMutableArray alloc] init] autorelease];
		for (int c=1;c<argc;c++)
		{
			[args addObject:[NSString stringWithFormat:@"%s",argv[c]]];
		}
		NSString *dylibPath = [NSString stringWithFormat:@"%@:%s/WebKit.framework/Versions/Current",libpath,runtimePath.c_str()];
		dylibPath = [dylibPath stringByAppendingFormat:@":%s/WebCore.framework/Versions/Current",runtimePath.c_str()];
		dylibPath = [dylibPath stringByAppendingFormat:@":%s/JavaScriptCore.framework/Versions/Current",runtimePath.c_str()];
		dylibPath = [dylibPath stringByAppendingFormat:@":%s",runtimePath.c_str()];
	
		// create our environment
		NSMutableDictionary *env = [[NSMutableDictionary alloc] init];
		[env setValue:appDir forKey:@"KR_HOME"];
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
	
		[invoker initWithApplication:runtimeExec environment:env arguments:args currentDirectory:bundlePath];
		[invoker launch:terminateInvocation];
	}
	
	[pool release];
	return 0;
}
