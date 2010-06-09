/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
namespace BootUtils
{
	// These are also used in application.cpp
	void ScanBundledComponents(string, vector<SharedComponent>&);

	static void ScanRuntimesAtPath(string, vector<SharedComponent>&, bool=true);
	static void ScanModulesAtPath(string, vector<SharedComponent>&, bool=true);
	static void ScanSDKsAtPath(string, vector<SharedComponent>&, bool=true);
	static void ScanMobileSDKsAtPath(string, vector<SharedComponent>&, bool=true);
	static void AddToComponentVector(vector<SharedComponent>&, SharedComponent);

	static void AddToComponentVector(vector<SharedComponent>& components,
		SharedComponent c)
	{
		// Avoid adding duplicate components to a component vector
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent e = *i++;
			if (e->type == c->type && e->path == c->path)
			{
				return;
			}
		}

		components.push_back(c);
	}

	vector<SharedComponent>& GetInstalledComponents(bool force)
	{
		static std::vector<SharedComponent> installedComponents;
		if (installedComponents.empty() || force)
		{
			installedComponents.clear();
			vector<string>& paths = GetComponentSearchPaths();
			vector<string>::iterator i = paths.begin();
			while (i != paths.end())
			{
				string path(*i++);
				ScanRuntimesAtPath(path, installedComponents, false);
				ScanSDKsAtPath(path, installedComponents, false);
				ScanMobileSDKsAtPath(path, installedComponents, false);
				ScanModulesAtPath(path, installedComponents, false);
			}

			// Sort components by version here so that the latest version of
			// any component will always be chosen. Use a stable_sort because we
			// want to give preference to components earlier on the search path.
			std::stable_sort(
				installedComponents.begin(),
				installedComponents.end(),
				BootUtils::WeakCompareComponents);
		}
		return installedComponents;
	}

	class PathBits
	{
	public:
		PathBits(const string& name, const string& fullPath) :
			name(name),
			fullPath(fullPath)
		{ }
		std::string name;
		std::string fullPath;
	};

	static vector<PathBits> GetDirectoriesAtPath(std::string& path)
	{
		vector<PathBits> directories;
		vector<string> paths;

		FileUtils::ListDir(path, paths);
		vector<string>::iterator i = paths.begin();
		while (i != paths.end())
		{
			string& subpath(*i++);
			if (subpath[0] == '.')
				continue;

			string fullPath(FileUtils::Join(path.c_str(), subpath.c_str(), NULL));
			if (!FileUtils::IsDirectory(fullPath))
				continue;

			directories.push_back(PathBits(subpath, fullPath));
		}
		return directories;
	}

	static void ScanRuntimesAtPath(string path, vector<SharedComponent>& results, bool bundled)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		// Read everything that looks like <searchpath>/runtime/<os>/*
		string rtPath(FileUtils::Join(path.c_str(), "runtime", 0));
		if (!bundled)
			rtPath = FileUtils::Join(rtPath.c_str(), OS_NAME, 0);
		vector<PathBits> versions(GetDirectoriesAtPath(rtPath));
		for (size_t i = 0; i < versions.size(); i++)
		{
			PathBits& b = versions[i];
			AddToComponentVector(results,
				KComponent::NewComponent(RUNTIME, "runtime", b.name, b.fullPath));
		}
	}

	static void ScanSDKsAtPath(string path, vector<SharedComponent>& results, bool bundled)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		// Read everything that looks like <searchpath>/sdk/<os>/*
		string sdkPath(FileUtils::Join(path.c_str(), "sdk", 0));
		if (!bundled)
			sdkPath = FileUtils::Join(sdkPath.c_str(), OS_NAME, 0);
		vector<PathBits> versions(GetDirectoriesAtPath(sdkPath));

		for (size_t i = 0; i < versions.size(); i++)
		{
			PathBits& b = versions[i];
			AddToComponentVector(results,
				KComponent::NewComponent(SDK, "sdk", b.name, b.fullPath, bundled));
		}
	}

	static void ScanMobileSDKsAtPath(string path, vector<SharedComponent>& results, bool bundled)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		// Read everything that looks like <searchpath>/mobilesdk/<os>/*
		string sdkPath(FileUtils::Join(path.c_str(), "mobilesdk", 0));
		if (!bundled)
			sdkPath = FileUtils::Join(sdkPath.c_str(), OS_NAME, 0);
		vector<PathBits> versions(GetDirectoriesAtPath(sdkPath));

		for (size_t i = 0; i < versions.size(); i++)
		{
			PathBits& b = versions[i];
			AddToComponentVector(results,
				KComponent::NewComponent(MOBILESDK, "mobilesdk", b.name, b.fullPath, bundled));
		}
	}

	static void ScanModulesAtPath(string path, vector<SharedComponent>& results, bool bundled)
	{
		if (!FileUtils::IsDirectory(path))
			return;

		// Read everything that looks like <searchpath>/modules/<os>/*
		string namesPath(FileUtils::Join(path.c_str(), "modules", 0));
		if (!bundled)
			namesPath = FileUtils::Join(namesPath.c_str(), OS_NAME, 0);
		vector<PathBits> moduleNames(GetDirectoriesAtPath(namesPath));

		for (size_t i = 0; i < moduleNames.size(); i++)
		{
			PathBits& moduleName = moduleNames[i];

			// Read everything that looks like <searchpath>/modules/<os>/<name>/*
			vector<PathBits> moduleVersions(GetDirectoriesAtPath(moduleName.fullPath));
			for (size_t j = 0; j < moduleVersions.size(); j++)
			{
				PathBits& moduleVersion = moduleVersions[j];
				AddToComponentVector(results, KComponent::NewComponent(
					MODULE, moduleName.name, moduleVersion.name, moduleVersion.fullPath, bundled));
			}
		}
	}

	void ScanBundledComponents(string path, vector<SharedComponent>& results)
	{
		ScanRuntimesAtPath(path, results, true);
		ScanMobileSDKsAtPath(path, results, true);
		ScanSDKsAtPath(path, results, true);
		ScanModulesAtPath(path, results, true);
	}

	int CompareVersions(string one, string two)
	{
		if (one.empty() && two.empty())
			return 0;
		if (one.empty())
			return -1;
		if (two.empty())
			return 1;

		vector<string> listOne;
		vector<string> listTwo;
		FileUtils::Tokenize(one, listOne, ".");
		FileUtils::Tokenize(two, listTwo, ".");

		size_t min = listOne.size();
		if (listTwo.size() < listOne.size())
			min = listTwo.size();

		for (size_t i = 0; i < min; i++)
		{
			int result = listOne.at(i).compare(listTwo.at(i));
			if (result != 0)
				return result;
		}

		if (listOne.size() > listTwo.size())
			return 1;
		else if (listTwo.size() > listOne.size())
			return -1;
		else
			return 0;
	}

	bool WeakCompareComponents(SharedComponent one, SharedComponent two)
	{
		return BootUtils::CompareVersions(one->version, two->version) > 0;
	}

	vector<pair<string, string> > ReadManifestFile(std::string path)
	{
		vector<pair<string, string> > manifest;
		if (!FileUtils::IsFile(path))
			return manifest;

		string manifestContents(FileUtils::ReadFile(path));
		if (manifestContents.empty())
			return manifest;

		vector<string> manifestLines;
		FileUtils::Tokenize(manifestContents, manifestLines, "\n");
		for (size_t i = 0; i < manifestLines.size(); i++)
		{
			string line = FileUtils::Trim(manifestLines[i]);

			size_t pos = line.find(":");
			if (pos == 0 || pos == line.length() - 1)
			{
				continue;
			}
			else
			{
				manifest.push_back(pair<string, string>(
					FileUtils::Trim(line.substr(0, pos)), // The key
					FileUtils::Trim(line.substr(pos + 1, line.length())))); // The value.
			}
		}
		return manifest;
	}

	SharedComponent ResolveDependency(SharedDependency dep, vector<SharedComponent>& components)
	{
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent comp = *i++;
			if (dep->type != comp->type || dep->name != comp->name)
				continue;

			int compare = CompareVersions(comp->version, dep->version);
			if ((dep->requirement == Dependency::EQ && compare == 0)
				|| (dep->requirement == Dependency::GTE && compare >= 0)
				|| (dep->requirement == Dependency::GT && compare > 0)
				|| (dep->requirement == Dependency::LT && compare < 0))
			{
				return comp;
			}
		}

		return NULL;
	} 
}
	SharedDependency Dependency::NewDependencyFromValues(
		KComponentType type, std::string name, std::string version)
	{
		Dependency* d = new Dependency();
		d->type = type;
		d->name = name;
		d->version = version;
		d->requirement = EQ;
		return d;
	}

	SharedDependency Dependency::NewDependencyFromManifestLine(
		string key, string value)
	{
		Dependency* d = new Dependency();
		size_t versionStart;
		if (value.find(">=") != string::npos)
		{
			d->requirement = GTE;
			versionStart = 2;
		}
		else if (value.find("<=") != string::npos)
		{
			d->requirement = LTE;
			versionStart = 2;
		}
		else if (value.find("<") != string::npos)
		{
			d->requirement = LT;
			versionStart = 1;
		}
		else if (value.find(">") != string::npos)
		{
			d->requirement = GT;
			versionStart = 1;
		}
		else if (value.find("=") != string::npos)
		{
			d->requirement = EQ;
			versionStart = 1;
		}
		else
		{
			d->requirement = EQ;
			versionStart = 0;
		}

		d->name = key;
		d->version = value.substr(versionStart);

		if (key == "runtime")
		{
			d->type = RUNTIME;
		}
		else if (key == "sdk")
		{
			d->type = SDK;
		}
		else if (key == "mobilesdk")
		{
			d->type = MOBILESDK;
		}
		else
		{
			d->type = MODULE;
		}
		return d;
	}

	SharedComponent KComponent::NewComponent(KComponentType type, string name,
		string version, string path, bool bundled)
	{
		KComponent* c = new KComponent();
		c->type = type;
		c->name = name;
		c->version = version;
		c->path = path;
		c->bundled = true;
		return c;
	}
}
