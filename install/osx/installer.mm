/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Foundation/Foundation.h>
#import <api/file_utils.h>

using namespace kroll;

/**
 * this program has one single purpose: given a source, install 
 * kroll components (runtime, modules, etc) into the destination
 * directory
 */
int main(int argc, char* argv[])
{
	if (argc!=3)
	{
		fprintf(stderr,"Invalid arguments passed\n");
		return __LINE__;
	}
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *source = [NSString stringWithFormat:@"%s",argv[1]];
	NSString *dest = [NSString stringWithFormat:@"%s",argv[2]];
	
	// make sure installer exists - paranoid check
	std::string ss = std::string([source UTF8String]);
	if (!FileUtils::IsDirectory(ss))
	{
		fprintf(stderr,"invalid argument passed. installer dir: %s doesn't exist\n",[source UTF8String]);
		[pool release];
		return __LINE__;
	}
	
	// copy files
	NSDirectoryEnumerator *dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:source];
	NSString *file;
	while ((file = [dirEnum nextObject]))
	{
	    if ([[file pathExtension] isEqualToString: @"zip"]) 
		{
			NSArray *parts = [file componentsSeparatedByString:@"-"];
			if ([parts count] == 3)
			{
				NSString *type = [parts objectAtIndex:0];
				NSString *subtype = [parts objectAtIndex:1];
				NSString *version = [[parts objectAtIndex:2] stringByDeletingPathExtension];
				/**
				 * directories:
				 *
				 * /runtime/<version>/<files>
				 * /modules/<name>/<version>
				 */
				NSString *sourceFile = [NSString stringWithFormat:@"%@/%@",source,file];
				NSString *dir = nil;
				if ([type isEqualToString:@"runtime"])
				{
					dir = [NSString stringWithFormat:@"%@/runtime/%@/%@",dest,subtype,version];
				}
				else if ([type isEqualToString:@"module"])
				{
					dir = [NSString stringWithFormat:@"%@/modules/%@/%@",dest,subtype,version];
				}
				if (dir)
				{
					[[NSFileManager defaultManager]createDirectoryAtPath:dir attributes:nil];
					std::string src([sourceFile UTF8String]);
					std::string dest([dir UTF8String]);
					FileUtils::Unzip(src,dest);
				}
			}
	    }
	}

	[pool release];
	return 0;
}
