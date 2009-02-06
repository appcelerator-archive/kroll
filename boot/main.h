/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#if defined(OS_OSX)
#import <Cocoa/Cocoa.h>
#endif

#if defined(OS_LINUX)
#include <gtk/gtk.h>
#endif

#if defined(OS_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <dlfcn.h>
#include <signal.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>

#include <api/file_utils.h> // this safe, but not the others

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

//
// these flags are compiled in to allow them
// to be tailed to the embedded environment
//
#ifndef _BOOT_RUNTIME_FLAG
  #define _BOOT_RUNTIME_FLAG --kruntime
#endif

#ifndef _BOOT_HOME_FLAG
  #define _BOOT_HOME_FLAG --khome
#endif

#ifndef _BOOT_UPDATESITE_ENVNAME
  #define _BOOT_UPDATESITE_ENVNAME UPDATESITE
#endif

#ifndef _BOOT_UPDATESITE_URL
  #define _BOOT_UPDATESITE_URL   // we don't provide a default
#endif

#ifndef BOOT_RUNTIME_FLAG
  #define BOOT_RUNTIME_FLAG STRING(_BOOT_RUNTIME_FLAG)
#endif

#ifndef BOOT_HOME_FLAG
  #define BOOT_HOME_FLAG STRING(_BOOT_HOME_FLAG)
#endif

#ifndef BOOT_UPDATESITE_ENVNAME
  #define BOOT_UPDATESITE_ENVNAME STRING(_BOOT_UPDATESITE_ENVNAME)
#endif

#ifndef BOOT_UPDATESITE_URL
  #define BOOT_UPDATESITE_URL STRING(_BOOT_UPDATESITE_URL)
#endif

#ifndef OS_NAME
  #ifndef _OS_NAME
  	#error _OS_NAME should have been defined!
  #endif
  #define OS_NAME STRING(_OS_NAME)
#endif

#ifdef OS_OSX
  #define KR_FATAL_ERROR(msg) \
  { \
	[NSApplication sharedApplication]; \
	NSAlert *alert = [[NSAlert alloc] init]; \
	[alert addButtonWithTitle:@"OK"]; \
	[alert setMessageText:@"Application Error"]; \
	[alert setInformativeText:[NSString stringWithCString:msg]]; \
	[alert setAlertStyle:NSCriticalAlertStyle]; \
	[alert runModal]; \
	[alert release]; \
	 \
  }
#elif OS_WIN32
  #define KR_FATAL_ERROR(msg) \
  { \
	MessageBox(NULL,msg,"Application Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL); \
  }
#elif OS_LINUX
  #define KR_FATAL_ERROR(msg) \
{ \
	GtkWidget* dialog = gtk_message_dialog_new(\
		NULL,  \
		GTK_DIALOG_MODAL, \
		GTK_MESSAGE_ERROR,  \
		GTK_BUTTONS_OK, \
		"%s", \
		msg); \
	gtk_dialog_run(GTK_DIALOG(dialog)); \
	gtk_widget_destroy(dialog); \
}
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
