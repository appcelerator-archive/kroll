/**
 * Useful OSX specific code borrowed from the Google Mac Toolkit
 * (see LICENSE below) with minor renames/removal/compiler fixes
 * re-licensed under the same license by Appcelerator
 */
#include "../base.h"
#include "nslog_channel.h"

#import <Foundation/Foundation.h>

// 
// GTMDefines.h
//
//  Copyright 2008 Google Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not
//  use this file except in compliance with the License.  You may obtain a copy
//  of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
//  License for the specific language governing permissions and limitations under
//  the License.
//
 
// ============================================================================

#include <AvailabilityMacros.h>

// Not all MAC_OS_X_VERSION_10_X macros defined in past SDKs
#ifndef MAC_OS_X_VERSION_10_5
  #define MAC_OS_X_VERSION_10_5 1050
#endif
#ifndef MAC_OS_X_VERSION_10_6
  #define MAC_OS_X_VERSION_10_6 1060
#endif

// ----------------------------------------------------------------------------
// CPP symbols that can be overridden in a prefix to control how the toolbox
// is compiled.
// ----------------------------------------------------------------------------

// Give ourselves a consistent way to do inlines.  Apple's macros even use
// a few different actual definitions, so we're based off of the foundation
// one.
#if !defined(GTM_INLINE)
  #if defined (__GNUC__) && (__GNUC__ == 4)
    #define GTM_INLINE static __inline__ __attribute__((always_inline))
  #else
    #define GTM_INLINE static __inline__
  #endif
#endif

// Give ourselves a consistent way of doing externs that links up nicely
// when mixing objc and objc++
#if !defined (GTM_EXTERN)
  #if defined __cplusplus
    #define GTM_EXTERN extern "C"
  #else
    #define GTM_EXTERN extern
  #endif
#endif


// ============================================================================

// ----------------------------------------------------------------------------
// CPP symbols defined based on the project settings so the GTM code has
// simple things to test against w/o scattering the knowledge of project
// setting through all the code.
// ----------------------------------------------------------------------------

// To simplify support for 64bit (and Leopard in general), we provide the type
// defines for non Leopard SDKs
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
 // NSInteger/NSUInteger and Max/Mins
  #ifndef NSINTEGER_DEFINED
    #if __LP64__ || NS_BUILD_32_LIKE_64
      typedef long NSInteger;
      typedef unsigned long NSUInteger;
    #else
      typedef int NSInteger;
      typedef unsigned int NSUInteger;
    #endif
    #define NSIntegerMax    LONG_MAX
    #define NSIntegerMin    LONG_MIN
    #define NSUIntegerMax   ULONG_MAX
    #define NSINTEGER_DEFINED 1
  #endif  // NSINTEGER_DEFINED
  // CGFloat
  #ifndef CGFLOAT_DEFINED
    #if defined(__LP64__) && __LP64__
      // This really is an untested path (64bit on Tiger?)
      typedef double CGFloat;
      #define CGFLOAT_MIN DBL_MIN
      #define CGFLOAT_MAX DBL_MAX
      #define CGFLOAT_IS_DOUBLE 1
    #else /* !defined(__LP64__) || !__LP64__ */
      typedef float CGFloat;
      #define CGFLOAT_MIN FLT_MIN
      #define CGFLOAT_MAX FLT_MAX
      #define CGFLOAT_IS_DOUBLE 0
    #endif /* !defined(__LP64__) || !__LP64__ */
    #define CGFLOAT_DEFINED 1
  #endif // CGFLOAT_DEFINED
#endif  // MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4


// Returns a string containing a nicely formatted stack trace.
// 
// This function gets the stack trace for the current thread. It will
// be from the caller of GTMStackTrace upwards to the top the calling stack.
// Typically this function will be used along with some logging, 
// as in the following:
//
//   MyAppLogger(@"Should never get here:\n%@", GTMStackTrace());
//
// Here is a sample stack trace returned from this function:
//
// #0  0x00002d92 D ()  [/Users/me/./StackLog]
// #1  0x00002e45 C ()  [/Users/me/./StackLog]
// #2  0x00002e53 B ()  [/Users/me/./StackLog]
// #3  0x00002e61 A ()  [/Users/me/./StackLog]
// #4  0x00002e6f main ()  [/Users/me/./StackLog]
// #5  0x00002692 tart ()  [/Users/me/./StackLog]
// #6  0x000025b9 tart ()  [/Users/me/./StackLog]
//

KROLL_API NSString *KrollStackTrace(void);
KROLL_API void KrollDumpStackTrace(); 

KROLL_API NSString *KrollStackTraceFromException(NSException *e);
KROLL_API void KrollDumpStackTraceFromException(NSException* e);

#ifdef OSX_CPP_COMPILE

//
//  GTMObjC2Runtime.h
//
//  Copyright 2007-2008 Google Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not
//  use this file except in compliance with the License.  You may obtain a copy
//  of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
//  License for the specific language governing permissions and limitations under
//  the License.
//

#import <objc/objc-api.h>

// These functions exist for code that we want to compile on both the < 10.5 
// sdks and on the >= 10.5 sdks without warnings. It basically reimplements 
// certain parts of the objc2 runtime in terms of the objc1 runtime. It is not 
// a complete implementation as I've only implemented the routines I know we 
// use. Feel free to add more as necessary.
// These functions are not documented because they conform to the documentation
// for the ObjC2 Runtime.

#if OBJC_API_VERSION >= 2  // Only have optional and req'd keywords in ObjC2.
#define AT_OPTIONAL @optional
#define AT_REQUIRED @required
#else
#define AT_OPTIONAL
#define AT_REQUIRED
#endif

// The file objc-runtime.h was moved to runtime.h and in Leopard, objc-runtime.h 
// was just a wrapper around runtime.h. For the iPhone SDK, this objc-runtime.h
// is removed in the iPhoneOS2.0 SDK.
//
// The |Object| class was removed in the iPhone2.0 SDK too.
#if GTM_IPHONE_SDK
#import <objc/runtime.h>
#else
#import <objc/objc-runtime.h>
#import <objc/Object.h>
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
#import "objc/Protocol.h"
#import <libkern/OSAtomic.h>

OBJC_EXPORT Class object_getClass(id obj);
OBJC_EXPORT const char *class_getName(Class cls);
OBJC_EXPORT BOOL class_conformsToProtocol(Class cls, Protocol *protocol);
OBJC_EXPORT Class class_getSuperclass(Class cls);
OBJC_EXPORT Method *class_copyMethodList(Class cls, unsigned int *outCount);
OBJC_EXPORT SEL method_getName(Method m);
OBJC_EXPORT void method_exchangeImplementations(Method m1, Method m2);
OBJC_EXPORT IMP method_getImplementation(Method method);
OBJC_EXPORT IMP method_setImplementation(Method method, IMP imp);
OBJC_EXPORT struct objc_method_description protocol_getMethodDescription(Protocol *p,
                                                                         SEL aSel,
                                                                         BOOL isRequiredMethod,
                                                                         BOOL isInstanceMethod);

// If building for 10.4 but using the 10.5 SDK, don't include these.
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
// atomics
// On Leopard these are GC aware
// Intentionally did not include the non-barrier versions, because I couldn't
// come up with a case personally where you wouldn't want to use the
// barrier versions.
GTM_INLINE bool OSAtomicCompareAndSwapPtrBarrier(void *predicate,
                                                 void *replacement,
                                                 volatile void *theValue) {
#if defined(__LP64__) && __LP64__
  return OSAtomicCompareAndSwap64Barrier((int64_t)predicate,
                                         (int64_t)replacement,
                                         (int64_t *)theValue);
#else  // defined(__LP64__) && __LP64__
  return OSAtomicCompareAndSwap32Barrier((int32_t)predicate,
                                         (int32_t)replacement,
                                         (int32_t *)theValue);
#endif  // defined(__LP64__) && __LP64__
}
  
GTM_INLINE BOOL objc_atomicCompareAndSwapGlobalBarrier(id predicate, 
                                                       id replacement, 
                                                       volatile id *objectLocation) {
  return OSAtomicCompareAndSwapPtrBarrier(predicate, 
                                          replacement, 
                                          objectLocation);
}
GTM_INLINE BOOL objc_atomicCompareAndSwapInstanceVariableBarrier(id predicate, 
                                                                 id replacement, 
                                                                 volatile id *objectLocation) {
  return OSAtomicCompareAndSwapPtrBarrier(predicate, 
                                          replacement, 
                                          objectLocation);
}
#endif

#endif  // OBJC2_UNAVAILABLE


//
//  GTMStackTrace.h
//
//  Copyright 2007-2008 Google Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not
//  use this file except in compliance with the License.  You may obtain a copy
//  of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
//  License for the specific language governing permissions and limitations under
//  the License.
//

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GTMAddressDescriptor {
  const void *address;  // address
  const char *symbol;  // nearest symbol to address
  const char *class_name;  // if it is an obj-c method, the method's class
  BOOL is_class_method;  // if it is an obj-c method, type of method
  const char *filename;  // file that the method came from.
};


#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
// Returns an array of program counters from the current thread's stack.
// *** You should probably use GTMStackTrace() instead of this function ***
// However, if you actually want all the PCs in "void *" form, then this
// funtion is more convenient. This will include PCs of GTMStaceTrace and
// its inner utility functions that you may want to strip out.
//
// You can use +[NSThread callStackReturnAddresses] in 10.5 or later.
//
// Args:
//   outPcs - an array of "void *" pointers to the program counters found on the
//            current thread's stack.
//   count - the number of entries in the outPcs array
//
// Returns:
//   The number of program counters actually added to outPcs.
//
NSUInteger GTMGetStackProgramCounters(void *outPcs[], NSUInteger count);
#endif  // MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5

// Returns an array of GTMAddressDescriptors from the current thread's stack.
// *** You should probably use GTMStackTrace() instead of this function ***
// However, if you actually want all the PCs with symbols, this is the way
// to get them. There is no memory allocations done, so no clean up is required
// except for the caller to free outDescs if they allocated it themselves.
// This will include PCs of GTMStaceTrace and its inner utility functions that 
// you may want to strip out.
//
// Args:
//   outDescs - an array of "struct GTMAddressDescriptor" pointers corresponding
//              to the program counters found on the current thread's stack.
//   count - the number of entries in the outDescs array
//
// Returns:
//   The number of program counters actually added to outPcs.
//
NSUInteger GTMGetStackAddressDescriptors(struct GTMAddressDescriptor outDescs[], 
                                         NSUInteger count);

#ifdef __cplusplus
}

#endif

#endif


