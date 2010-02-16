#define OSX_CPP_COMPILE
#import "osx.h"

//
//  GTMObjC2Runtime.m
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
#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
#import <stdlib.h>
#import <string.h>

Class object_getClass(id obj) {
  if (!obj) return NULL;
  return obj->isa;
}

const char *class_getName(Class cls) {
  if (!cls) return "nil";
  return cls->name;
}

BOOL class_conformsToProtocol(Class cls, Protocol *protocol) {
  // We intentionally don't check cls as it crashes on Leopard so we want
  // to crash on Tiger as well.
  // I logged
  // Radar 5572978 class_conformsToProtocol crashes when arg1 is passed as nil
  // because it seems odd that this API won't accept nil for cls considering
  // all the other apis will accept nil args.
  // If this does get fixed, remember to enable the unit tests.
  if (!protocol) return NO;

  struct objc_protocol_list *protos;
  for (protos = cls->protocols; protos != NULL; protos = protos->next) {
    for (long i = 0; i < protos->count; i++) {
      if ([protos->list[i] conformsTo:protocol]) {
        return YES;
      }
    }
  }
  return NO;
}

Class class_getSuperclass(Class cls) {
  if (!cls) return NULL;
  return cls->super_class;
}

Method *class_copyMethodList(Class cls, unsigned int *outCount) {
  if (!cls) return NULL;

  unsigned int count = 0;
  void *iterator = NULL;
  struct objc_method_list *mlist;
  Method *methods = NULL;
  if (outCount) *outCount = 0;

  while ( (mlist = class_nextMethodList(cls, &iterator)) ) {
    if (mlist->method_count == 0) continue;
    methods = (Method *)realloc(methods, 
                                sizeof(Method) * (count + mlist->method_count + 1));
    if (!methods) {
      //Memory alloc failed, so what can we do?
      return NULL;  // COV_NF_LINE
    }
    for (int i = 0; i < mlist->method_count; i++) {
      methods[i + count] = &mlist->method_list[i];
    }
    count += mlist->method_count;
  }

  // List must be NULL terminated
  if (methods) {
    methods[count] = NULL;
  }
  if (outCount) *outCount = count;
  return methods;
}

SEL method_getName(Method method) {
  if (!method) return NULL;
  return method->method_name;
}

IMP method_getImplementation(Method method) {
  if (!method) return NULL;
  return method->method_imp;
}

IMP method_setImplementation(Method method, IMP imp) {
  // We intentionally don't test method for nil.
  // Leopard fails here, so should we.
  // I logged this as Radar:
  // 5572981 method_setImplementation crashes if you pass nil for the 
  // method arg (arg 1)
  // because it seems odd that this API won't accept nil for method considering
  // all the other apis will accept nil args.
  // If this does get fixed, remember to enable the unit tests.
  // This method works differently on SnowLeopard than
  // on Leopard. If you pass in a nil for IMP on SnowLeopard
  // it doesn't change anything. On Leopard it will. Since
  // attempting to change a sel to nil is probably an error
  // we follow the SnowLeopard way of doing things.
  IMP oldImp = NULL;
  if (imp) {
    oldImp = method->method_imp;
    method->method_imp = imp;
  } 
  return oldImp;
}

void method_exchangeImplementations(Method m1, Method m2) {
  if (m1 == m2) return;
  if (!m1 || !m2) return;
  IMP imp2 = method_getImplementation(m2);
  IMP imp1 = method_setImplementation(m1, imp2);
  method_setImplementation(m2, imp1);
}

struct objc_method_description protocol_getMethodDescription(Protocol *p,
                                                             SEL aSel,
                                                             BOOL isRequiredMethod,
                                                             BOOL isInstanceMethod) {
  struct objc_method_description *descPtr = NULL;
  // No such thing as required in ObjC1.
  if (isInstanceMethod) {
    descPtr = [p descriptionForInstanceMethod:aSel];
  } else {
    descPtr = [p descriptionForClassMethod:aSel];
  }

  struct objc_method_description desc;
  if (descPtr) {
    desc = *descPtr;
  } else {
    bzero(&desc, sizeof(desc));
  }
  return desc;
}


#endif




//
//  GTMStackTrace.m
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

#include <stdlib.h>
#include <dlfcn.h>
#include <mach-o/nlist.h>

struct GTMClassDescription {
  const char *class_name;
  Method *class_methods;
  unsigned int class_method_count;
  Method *instance_methods;
  unsigned int instance_method_count;
};

#pragma mark Private utility functions

static struct GTMClassDescription *GTMClassDescriptions(NSUInteger *total_count) {
  int class_count = objc_getClassList(nil, 0);
  struct GTMClassDescription *class_descs = (GTMClassDescription*)calloc(class_count, sizeof(struct GTMClassDescription));
  if (class_descs) {
    Class *classes = (Class*)calloc(class_count, sizeof(Class));
    if (classes) {
      objc_getClassList(classes, class_count);
      for (int i = 0; i < class_count; ++i) {
        class_descs[i].class_methods 
          = class_copyMethodList(object_getClass(classes[i]), 
                                 &class_descs[i].class_method_count);
        class_descs[i].instance_methods 
          = class_copyMethodList(classes[i], 
                                 &class_descs[i].instance_method_count);
        class_descs[i].class_name = class_getName(classes[i]);
      }
      free(classes);
    } else {
      // COV_NF_START - Don't know how to force this in a unittest
      free(class_descs);
      class_count = 0;
      // COV_NF_END
    }
  }
  if (total_count) {
    *total_count = class_count;
  }
  return class_descs;
}

static void GTMFreeClassDescriptions(struct GTMClassDescription *class_descs, 
                                     NSUInteger count) {
  if (!class_descs) return;
  for (NSUInteger i = 0; i < count; ++i) {
    if (class_descs[i].instance_methods) {
      free(class_descs[i].instance_methods);
    }
    if (class_descs[i].class_methods) {
      free(class_descs[i].class_methods);
    }
  }
  free(class_descs);
}

static NSUInteger GTMGetStackAddressDescriptorsForAddresses(void *pcs[],
                                                            struct GTMAddressDescriptor outDescs[],
                                                            NSUInteger count) {
  if (count < 1 || !pcs || !outDescs) return 0;
  
  NSUInteger class_desc_count;
  
  // Get our obj-c class descriptions. This is expensive, so we do it once
  // at the top. We go through this because dladdr doesn't work with
  // obj methods.
  struct GTMClassDescription *class_descs 
    = GTMClassDescriptions(&class_desc_count);
  
  // Iterate through the stack.
  for (NSUInteger i = 0; i < count; ++i) {
    const char *class_name = NULL;
    Boolean is_class_method = FALSE;
    size_t smallest_diff = SIZE_MAX;
    struct GTMAddressDescriptor *currDesc = &outDescs[i];
    currDesc->address = pcs[i];
    Method best_method = NULL;
    // Iterate through all the classes we know of.
    for (NSUInteger j = 0; j < class_desc_count; ++j) {
      // First check the class methods.
      for (NSUInteger k = 0; k < class_descs[j].class_method_count; ++k) {
        IMP imp = method_getImplementation(class_descs[j].class_methods[k]);
        if (imp <= (IMP)currDesc->address) {
          size_t diff = (size_t)currDesc->address - (size_t)imp;
          if (diff < smallest_diff) {
            best_method = class_descs[j].class_methods[k];
            class_name = class_descs[j].class_name;
            is_class_method = TRUE;
            smallest_diff = diff;
          }
        }
      }
      // Then check the instance methods.
      for (NSUInteger k = 0; k < class_descs[j].instance_method_count; ++k) {
        IMP imp = method_getImplementation(class_descs[j].instance_methods[k]);
        if (imp <= (IMP)currDesc->address) {
          size_t diff = (size_t)currDesc->address - (size_t)imp;
          if (diff < smallest_diff) {
            best_method = class_descs[j].instance_methods[k];
            class_name = class_descs[j].class_name;
            is_class_method = TRUE;
            smallest_diff = diff;
          }
        }
      }
    }
    
    // If we have one, store it off.
    if (best_method) {
      currDesc->symbol = sel_getName(method_getName(best_method));
      currDesc->is_class_method = is_class_method;
      currDesc->class_name = class_name;
    }
    Dl_info info = { NULL, NULL, NULL, NULL };
    
    // Check to see if the one returned by dladdr is better.
    dladdr(currDesc->address, &info);
    if ((size_t)currDesc->address - (size_t)info.dli_saddr < smallest_diff) {
      currDesc->symbol = info.dli_sname;
      currDesc->is_class_method = FALSE;
      currDesc->class_name = NULL;
    }
    currDesc->filename = info.dli_fname;
  }
  GTMFreeClassDescriptions(class_descs, class_desc_count);
  return count;
}

static NSString *GTMStackTraceFromAddressDescriptors(struct GTMAddressDescriptor descs[],
                                                     NSUInteger count) {
  NSMutableString *trace = [NSMutableString string];
  
  for (NSUInteger i = 0; i < count; i++) {
    // Newline between all the lines
    if (i) {
      [trace appendString:@"\n"];
    }
    if (descs[i].class_name) {
      [trace appendFormat:@"#%-2u %#08lx %s[%s %s]  (%s)",
       i, descs[i].address, 
       (descs[i].is_class_method ? "+" : "-"),
       descs[i].class_name,
       (descs[i].symbol ? descs[i].symbol : "??"),
       (descs[i].filename ? descs[i].filename : "??")];
    } else {
      [trace appendFormat:@"#%-2u %#08lx %s()  (%s)",
       i, descs[i].address,
       (descs[i].symbol ? descs[i].symbol : "??"),
       (descs[i].filename ? descs[i].filename : "??")];
    }
  }
  return trace;
}

#pragma mark Public functions

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
// Before 10.5, we have to do this ourselves.  10.5 adds
// +[NSThread callStackReturnAddresses].

// Structure representing a small portion of a stack, starting from the saved
// frame pointer, and continuing through the saved program counter.
struct GTMStackFrame {
  void *saved_fp;
#if defined (__ppc__) || defined(__ppc64__)
  void *padding;
#endif
  void *saved_pc;
};

// __builtin_frame_address(0) is a gcc builtin that returns a pointer to the
// current frame pointer.  We then use the frame pointer to walk the stack
// picking off program counters and other saved frame pointers.  This works
// great on i386, but PPC requires a little more work because the PC (or link
// register) isn't always stored on the stack.
//   
NSUInteger GTMGetStackProgramCounters(void *outPcs[], NSUInteger count) {
  if (!outPcs || (count < 1)) return 0;
  
  struct GTMStackFrame *fp;
#if defined (__ppc__) || defined(__ppc64__)
  outPcs[0] = __builtin_return_address(0);
  fp = (struct GTMStackFrame *)__builtin_frame_address(1);
#elif defined (__i386__) || defined(__x86_64__)
  fp = (struct GTMStackFrame *)__builtin_frame_address(0);
#else
#error architecture not supported
#endif

  NSUInteger level = 0;
  while (level < count) {
    if (fp == NULL) {
      level--;
      break;
    }
    outPcs[level] = fp->saved_pc;
    level++;
    fp = (struct GTMStackFrame *)fp->saved_fp;
  }
  
  return level;
}
#endif  // MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5

NSUInteger GTMGetStackAddressDescriptors(struct GTMAddressDescriptor outDescs[], 
                                         NSUInteger count) {
  if (count < 1 || !outDescs) return 0;
  NSUInteger result = 0;
  
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
  // Before 10.5, we collect the stack ourselves.

  void **pcs = calloc(count, sizeof(void*));
  if (!pcs) return 0;
  
  NSUInteger newSize = GTMGetStackProgramCounters(pcs, count);
  
  result = GTMGetStackAddressDescriptorsForAddresses(pcs, outDescs, newSize);
  free(pcs);

#else  //  MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
  // Use +[NSThread callStackReturnAddresses]
  
  NSArray *addresses = [NSThread callStackReturnAddresses];
  NSUInteger addrCount = [addresses count];
  if (addrCount) {
    void **pcs = (void**)calloc(addrCount, sizeof(void*));
    if (pcs) {
      void **pcsScanner = pcs;
      for (NSNumber *address in addresses) {
        NSUInteger addr = [address unsignedIntegerValue];
        *pcsScanner = (void *)addr;
        ++pcsScanner;
      }
      if (count < addrCount) {
        addrCount = count;
      }
      // Fill in the desc structures
      result = GTMGetStackAddressDescriptorsForAddresses(pcs, outDescs, addrCount);
    }
    if (pcs) free(pcs);
  }
#endif  // MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

  return result;
}

NSString *KrollStackTrace(void) {
  // If we don't have enough frames, return an empty string
  NSString *result = @"";
  
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
  // Before 10.5, we collect the stack ourselves.

  // The maximum number of stack frames that we will walk.  We limit this so
  // that super-duper recursive functions (or bugs) don't send us for an
  // infinite loop.
  struct GTMAddressDescriptor descs[100];
  size_t depth = sizeof(descs) / sizeof(struct GTMAddressDescriptor);
  depth = GTMGetStackAddressDescriptors(descs, depth);

  // Start at the second item so that GTMStackTrace and it's utility calls (of
  // which there is currently 1) is not included in the output.
  const size_t kTracesToStrip = 2;
  if (depth > kTracesToStrip) {
    result = GTMStackTraceFromAddressDescriptors(&descs[kTracesToStrip],
                                                 (depth - kTracesToStrip));
  }
#else  //  MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
  // Use +[NSThread callStackReturnAddresses]

  NSArray *addresses = [NSThread callStackReturnAddresses];
  NSUInteger count = [addresses count];
  if (count) {
    void **pcs = (void**) calloc(count, sizeof(void*));
    struct GTMAddressDescriptor *descs
      = (struct GTMAddressDescriptor*)calloc(count, sizeof(struct GTMAddressDescriptor));
    if (pcs && descs) {
      void **pcsScanner = pcs;
      for (NSNumber *address in addresses) {
        NSUInteger addr = [address unsignedIntegerValue];
        *pcsScanner = (void *)addr;
        ++pcsScanner;
      }
      // Fill in the desc structures
      count = GTMGetStackAddressDescriptorsForAddresses(pcs, descs, count);
      // Build the trace
      // We skip 1 frame because the +[NSThread callStackReturnAddresses] will
      // start w/ this frame.
      const size_t kTracesToStrip = 1;
      if (count > kTracesToStrip) {
        result = GTMStackTraceFromAddressDescriptors(&descs[kTracesToStrip],
                                                     (count - kTracesToStrip));
      }
    }
    if (pcs) free(pcs);
    if (descs) free(descs);
  }
#endif  // MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

  return result;
}

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

NSString *KrollStackTraceFromException(NSException *e) {
  NSString *trace = @"";
  
  // collect the addresses
  NSArray *addresses = [e callStackReturnAddresses];
  NSUInteger count = [addresses count];
  if (count) {
    void **pcs = (void**)calloc(count, sizeof(void*));
    struct GTMAddressDescriptor *descs
      = (struct GTMAddressDescriptor *)calloc(count, sizeof(struct GTMAddressDescriptor));
    if (pcs && descs) {
      void **pcsScanner = pcs;
      for (NSNumber *address in addresses) {
        NSUInteger addr = [address unsignedIntegerValue];
        *pcsScanner = (void *)addr;
        ++pcsScanner;
      }
      // Fill in the desc structures
      count = GTMGetStackAddressDescriptorsForAddresses(pcs, descs, count);
      // Build the trace
      trace = GTMStackTraceFromAddressDescriptors(descs, count);
    }
    if (pcs) free(pcs);
    if (descs) free(descs);
  }
  
  return trace;
}
#else

NSString *KrollStackTraceFromException(NSException *e)
{
	return @"<stacktrace not available in <10.5>";
}

#endif  // MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5



void KrollDumpStackTrace() {
	NSLog(@"%@",KrollStackTrace());
}

void KrollDumpStackTraceFromException(NSException *e){
	NSLog(@"%@",KrollStackTraceFromException(e));
}


#undef OSX_CPP_COMPILE
