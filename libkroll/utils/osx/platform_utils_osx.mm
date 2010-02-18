/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009-2010 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"
#include <Foundation/Foundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

namespace UTILS_NS
{
	void PlatformUtils::GetFirstMACAddressImpl(MACAddress& address)
	{
		CFMutableDictionaryRef matcher = IOServiceMatching(kIOEthernetInterfaceClass);
		if (!matcher)
			return;

		// Create a dictionary to match only the primary network interface.
		CFRef<CFMutableDictionaryRef> propertyMatcher(
			CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
		if (!propertyMatcher.get())
			return;
		CFDictionarySetValue(matcher, CFSTR(kIOPropertyMatchKey), propertyMatcher);

		io_iterator_t serviceIterator;
		if (IOServiceGetMatchingServices(kIOMasterPortDefault, matcher, &serviceIterator) != KERN_SUCCESS)
			return;

		// Grab the first result, which should be the primary interface.
		io_object_t service = IOIteratorNext(serviceIterator);
		if (!service)
			return;

		// Grab the parent of the first result, which should be the actual IONetworkController.
		io_object_t controller;
		if (IORegistryEntryGetParentEntry(service, kIOServicePlane, &controller) != KERN_SUCCESS)
			return;

		CFDataRef addressData = (CFDataRef) IORegistryEntryCreateCFProperty(
			controller, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
		if (!addressData)
		{
			IOObjectRelease(controller);
			IOObjectRelease(service);
			return;
		}

		UInt8 addressInts[kIOEthernetAddressSize];
		bzero(addressInts, sizeof(addressInts));

		CFDataGetBytes(addressData, CFRangeMake(0, MAC_ADDRESS_SIZE), addressInts);
		CFRelease(addressData);

		for (size_t i = 0; i < kIOEthernetAddressSize && i < MAC_ADDRESS_SIZE; i++)
			address[i] = addressInts[i];

		IOObjectRelease(controller);
		IOObjectRelease(service);
	}

	std::string PlatformUtils::GetUsername()
	{
		return std::string([NSUserName() UTF8String]);
	}

	int PlatformUtils::GetProcessorCount()
	{
		if (![NSProcessInfo instancesRespondToSelector:@selector(processorCount)])
			return 1; // Shucks!
	
		return [[NSProcessInfo processInfo] processorCount];
	}
}
