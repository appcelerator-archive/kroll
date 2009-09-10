/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#define _KROLL_BOOT_ 1
#ifndef _BOOT_H_

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

#define BOOTSTRAP_ENV "KR_BOOTSTRAPPED"
#define CRASH_REPORT_OPT "--crash_report"

#define CRASH_REPORT_URL  STRING(_CRASH_REPORT_URL)

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utils.h>

using namespace KrollUtils;
using KrollUtils::Application;
using KrollUtils::Dependency;
using KrollUtils::KComponent;
using KrollUtils::SharedApplication;
using KrollUtils::SharedDependency;
using KrollUtils::SharedComponent;
using std::string;
using std::vector;
using std::map;

#ifdef OS_WIN32
#define MODULE_SEPARATOR ";"
#else
#define MODULE_SEPARATOR ":"
#endif

namespace KrollBoot
{
	/**
	 * Implemented platform independently
	 */
	int Bootstrap();
	void FindUpdate();

	/**
	 * Implemented platform specifically
	 */
	void ShowError(std::string error, bool fatal=false);
	std::string GetApplicationHomePath();
	bool RunInstaller(vector<SharedDependency> missing, bool forceInstall=false);
	void BootstrapPlatformSpecific(string moduleList);
	int StartHost();
	string Blastoff();
	vector<SharedDependency> FilterForSDKInstall(
		vector<SharedDependency> dependencies);
	string GetApplicationName();

#ifdef USE_BREAKPAD
	void InitCrashDetection();
	string GetCrashDetectionTitle();
	string GetCrashDetectionHeader();
	string GetCrashDetectionMessage();
	map<string, string> GetCrashReportParameters();
#endif
}
#endif
