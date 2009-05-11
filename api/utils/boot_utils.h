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
#define SDK_UUID "FF71038E-3CD6-40EA-A1C2-CFEE1D284CEA"
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

namespace UTILS_NS
{
	using std::string;
	using std::vector;
	using std::pair;

	enum KComponentType
	{
		MODULE,
		RUNTIME,
		SDK,
		UNKNOWN
	};

	/**
	 * Represents a single component dependency -- one line in the application manifest
	 */
	class KROLL_API Dependency
	{
		public:
		enum Requirement
		{
			EQ,
			GT,
			LT,
			GTE,
			LTE,
		};
		KComponentType type;
		std::string name;
		std::string version;
		Requirement requirement;

		/**
		 * Generate a dependency from a key/value pair found in a manifest
		 */
		static SharedDependency NewDependency(std::string key, std::string value);
	};

	/**
	 * Represents a concrete Kroll components -- a runtime or module found on disk
	 */
	class KROLL_API KComponent
	{
		public:
		KComponentType type;
		std::string name;
		std::string version;
		std::string path;
		bool bundled;

		static SharedComponent NewComponent(
			KComponentType, std::string, std::string, std::string, bool bundled=false);

		/**
		 * Read a component's manifest. If there is no manifest
		 * file or it cannot be read return an empty vector.
		 */
		vector<pair<string, string> > ReadManifest();
	};

	class KROLL_API BootUtils
	{
		public:
		/**
		 * Compare two version strings in a piecewise way.
		 * @returns 1 if the first is larger, 0 if they are equal,
		 *     -1 if the second is larger
		 */
		static int CompareVersions(std::string, std::string);

		/**
		 * Compare two version strings in a piecewise way, weakly
		 * @returns true if the first is larger or false otherwise
		 */
		static bool WeakCompareComponents(SharedComponent, SharedComponent);

		/**
		 * Read a manifest file. 
		 * @returns a vector of key-value pairs which represent the 
		 *    manifest's contents or an empty vector if it cannot be read.
		 */
		static vector<pair<string, string> > ReadManifestFile(std::string);

		// These are lazily initialized static variables, so always
		// access them via the respective accessor functions.
		static std::vector<std::string> componentSearchPaths;
		static std::vector<std::string>& GetComponentSearchPaths();

		static std::vector<SharedComponent> installedComponents;
		static std::vector<SharedComponent>& GetInstalledComponents(bool force = false);
	};
}
#endif
