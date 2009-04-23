/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <sys/utsname.h>
#include <libgen.h>

using kroll::FileUtils;

std::string FileUtils::GetUserRuntimeHomeDirectory()
{
	NSString* nsPath = [
		NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, NO)
		objectAtIndex: 0];
	nsPath = [nsPath stringByAppendingPathComponent: [NSString stringWithUTF8String: PRODUCT_NAME]];
	return [[nsPath stringByExpandingTildeInPath] UTF8String];
}

std::string FileUtils::GetSystemRuntimeHomeDirectory()
{
	NSString* nsPath = [
		NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSLocalDomainMask, NO)
		objectAtIndex: 0];
	nsPath = [nsPath stringByAppendingPathComponent: [NSString stringWithUTF8String: PRODUCT_NAME]];
	return [[nsPath stringByExpandingTildeInPath] UTF8String];
}
