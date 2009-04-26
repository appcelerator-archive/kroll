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

	enum Requirement
	{
		EQ,
		GT,
		LT,
		GTE,
		LTE,
	};

	class KROLL_API KComponent
	{
		public:
		/*
		 * Construct a Kroll Component from the key-value pair found in a timanifest file.
		 */
		KComponent(std::string, std::string);

		/**
		 * Try to resolve a component by locating it in the given application or runtime homes
		 */
		bool Resolve(Application*, std::vector<std::string>& runtimeHomes);

		std::string GetURL(Application*);

		std::string type;
		std::string typeGuid;
		std::string name;
		std::string version;
		std::string path;
		Requirement requirement;

		private:
		static std::pair<Requirement, std::string> ParseVersion(std::string&);
	};

	class KROLL_API Application
	{
		public:
		~Application();

		/**
		 * Attempt to resolve all components that this applications contains.
		 * @returns a list of unresolved components
		 */
		std::vector<KComponent*> ResolveAllComponents(std::vector<std::string>& runtimeHomes);

		bool IsInstalled();
		std::string GetUpdateURL();
		std::string GetQueryString();
		std::string GetLicenseText();

		std::string path;
		std::string name;
		std::string version;
		std::string id;
		std::string guid;
		std::string publisher;
		std::string url;
		std::string image;
		std::vector<KComponent*> modules;
		KComponent* runtime;
		std::string runtimeHomePath;
		std::string queryString;
		std::string module_override;
		std::string runtime_override;
	};

	class KROLL_API BootUtils
	{
		public:

		/**
		 * Find a bundled .zip in a application directory
		 * @returns Path to the .zip or an empty string() if not found
		 */
		static std::string FindBundledModuleZip(
			std::string name,
			std::string version,
			std::string applicationDirectory);

		/**
		 * Find a subfolder which meets a version requirement 
		 * @returns Path to the subfolder with the greatest version matching the requirement
		 *          or an empty string() if none is found.
		 */
		static std::string FindVersionedSubfolder(
			std::string path,
			Requirement req,
			std::string version);

		static Application* ReadManifest(std::string applicationPath, int argc, char *argv[]);
		static Application* ReadManifestFile(std::string filePath, std::string appPath, int argc, char *argv[]);

		/**
		 * Compare two version strings in a piecewise way.
		 * @returns 1 if the first is larger, 0 if they are equal, -1 if the second is larger
		 */
		static int CompareVersions(std::string, std::string);

		/**
		 * Compare two version strings in a piecewise way, weakly
		 * @returns true if the first is larger or false otherwise
		 */
		static bool WeakCompareVersions(std::string, std::string);
	
		/** 
		 * Find a command line value (in the form --<name>=<value>) and return the value
		 * optionally de-quoting the value if it begins with "
		 * @returns value or default 
		 */
		static std::string FindCommandLineArg(std::string name, std::string def, int argc, char *argv[]);

		/** 
		 * Check a command line value (in the form --<name>) is present
		 * @returns true if found or false if not
		 */
		static bool HasCommandLineArg(std::string name, int argc, char *argv[]);
	};
}
#endif
