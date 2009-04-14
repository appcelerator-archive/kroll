/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BOOT_UTILS_H_
#define _KR_BOOT_UTILS_H_

#define DISTRIBUTION_URL "http://download.titaniumapp.com"
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
// these UUIDs should never change and uniquely identify a package type
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"
#define MANIFEST_FILENAME "manifest"
#define UPDATE_FILENAME ".update"
#define LICENSE_FILENAME "LICENSE.txt"
#define INSTALLED_MARKER_FILENAME ".installed"

// these flags are compiled in to allow them
// to be tailed to the embedded environment
#ifndef _BOOT_UPDATESITE_ENVNAME
  #define _BOOT_UPDATESITE_ENVNAME UPDATESITE
#endif

#ifndef BOOT_UPDATESITE_ENVNAME
  #define BOOT_UPDATESITE_ENVNAME STRING(_BOOT_UPDATESITE_ENVNAME)
#endif


namespace kroll
{
	class KComponent;
	class Application;

	class KROLL_API KComponent
	{
		public:
		/*
		 * Construct a Kroll Component from the key-value pair
		 * found in a timanifest file.
		 */
		KComponent(std::string, std::string);
		std::string GetURL(Application*);

		std::string type;
		std::string typeGuid;
		std::string name;
		std::string version;
		std::string path;
		int requirement;
	};

	class KROLL_API Application
	{
		public:
		~Application();
		bool IsInstalled();
		std::string GetQueryString();
		std::string GetLicenseText();

		std::string path;
		std::string name;
		std::string id;
		std::string guid;
		std::string publisher;
		std::string url;
		std::string image;
		std::vector<KComponent*> modules;
		KComponent* runtime;
		std::string queryString;
	};

	class KROLL_API BootUtils
	{
		public:
		static std::string FindBundledModuleZip(
			std::string name,
			std::string version,
			std::string applicationDirectory);

		static Application* ReadManifest(std::string applicationPath);
		static Application* ReadManifestFile(std::string filePath, std::string appPath);
	};
}
#endif
