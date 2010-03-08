/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_BOOT_UTILS_H_
#define _KR_BOOT_UTILS_H_

// These UUIDs should never change and uniquely identify a package type
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"
#define SDK_UUID "FF71038E-3CD6-40EA-A1C2-CFEE1D284CEA"
#define MOBILESDK_UUID "05645B49-C629-4D8F-93AF-F1CF83200E34"
#define APP_UPDATE_UUID "353AACB5-A36C-406A-8A67-33CD45E36550"
#define MANIFEST_FILENAME "manifest"
#define UPDATE_FILENAME ".update"
#define LICENSE_FILENAME "LICENSE.txt"
#define INSTALLED_MARKER_FILENAME ".installed"

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
		MOBILESDK,
		APP_UPDATE,
		UNKNOWN
	};

	/**
	 * Represents a single component dependency -- 
	 * one line in the application manifest
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
		static SharedDependency NewDependencyFromManifestLine(
			std::string key, std::string value);

		/**
		 * Generate a dependency from a set of values
		 */
		static SharedDependency NewDependencyFromValues(
			KComponentType type, std::string name, std::string version);
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

		static SharedComponent NewComponent(KComponentType type,
			std::string name, std::string version,
			std::string path, bool bundled=false);
	};

	namespace BootUtils
	{
		/**
		 * Compare two version strings in a piecewise way.
		 * @returns 1 if the first is larger, 0 if they are equal,
		 *     -1 if the second is larger
		 */
		KROLL_API int CompareVersions(std::string, std::string);

		/**
		 * Compare two version strings in a piecewise way, weakly
		 * @returns true if the first is larger or false otherwise
		 */
		KROLL_API bool WeakCompareComponents(SharedComponent, SharedComponent);

		/**
		 * Read a manifest file. 
		 * @returns a vector of key-value pairs which represent the 
		 *    manifest's contents or an empty vector if it cannot be read.
		 */
		KROLL_API vector<pair<string, string> > ReadManifestFile(std::string);

		/**
		 * Launch the intaller to install a list of dependencies. 
		 * @returns false only if the installer cannot be found
		 */
		KROLL_API bool RunInstaller(
			vector<SharedDependency> missing,
			SharedApplication application,
			std::string updatefile = "",
			std::string installerPath = "",
			bool quiet=false,
			bool forceInstaller=false);

		KROLL_API std::vector<std::string>& GetComponentSearchPaths();

		KROLL_API std::vector<SharedComponent>& GetInstalledComponents(
			bool force=false);
		
		KROLL_API SharedComponent ResolveDependency(SharedDependency dep, std::vector<SharedComponent>&);

	};
}
#endif
