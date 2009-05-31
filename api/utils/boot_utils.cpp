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
	vector<string> BootUtils::componentSearchPaths;
	vector<SharedComponent> BootUtils::installedComponents;

	void ScanRuntimesAtPath(string, vector<SharedComponent>&);
	void ScanSDKsAtPath(string, vector<SharedComponent>&);
	void ScanMobileSDKsAtPath(string, vector<SharedComponent>&);
	void ScanModulesAtPath(string, vector<SharedComponent>&);
	void ScanBundledComponents(string, vector<SharedComponent>&);
	void AddToComponentVector(vector<SharedComponent>&, SharedComponent);

	void AddToComponentVector(vector<SharedComponent>& components, SharedComponent c)
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

	vector<SharedComponent>& BootUtils::GetInstalledComponents(bool force)
	{
		static bool initialized = false;
		if (!initialized || force)
		{
			installedComponents.clear();
			vector<string>& paths = GetComponentSearchPaths();
			vector<string>::iterator i = paths.begin();
			while (i != paths.end())
			{
				string path = *i++;
				ScanRuntimesAtPath(path, installedComponents);
				ScanSDKsAtPath(path, installedComponents);
				ScanMobileSDKsAtPath(path, installedComponents);
				ScanModulesAtPath(path, installedComponents);
			}

			// Sort components by version here so that the latest version of
			// any component will always be chosen. Use a stable_sort because we
			// want to give preference to components earlier on the search path.
			std::stable_sort(
				installedComponents.begin(),
				installedComponents.end(),
				BootUtils::WeakCompareComponents);

			initialized = true;
		}
		return BootUtils::installedComponents;
	}

	void ScanRuntimesAtPath(string path, vector<SharedComponent>& results)
	{
		vector<string> paths;
		SharedComponent c;
		if (!FileUtils::IsDirectory(path))
		{
			return;
		}

		// Read everything that looks like <searchpath>/runtime/<os>/*
		string rtPath = FileUtils::Join(path.c_str(), "runtime", OS_NAME, NULL);
		FileUtils::ListDir(rtPath, paths);

		vector<string>::iterator runtimeVersion = paths.begin();
		while (runtimeVersion != paths.end())
		{
			string version = *runtimeVersion++;
			string fullPath = FileUtils::Join(rtPath.c_str(), version.c_str(), NULL);
			c = KComponent::NewComponent(RUNTIME, "runtime", version, fullPath);
			AddToComponentVector(results, c);
		}
	}

	void ScanSDKsAtPath(string path, vector<SharedComponent>& results)
	{
		vector<string> paths;
		SharedComponent c;
		if (!FileUtils::IsDirectory(path))
		{
			return;
		}

		// Read everything that looks like <searchpath>/sdk/<os>/*
		string sdkPath = FileUtils::Join(path.c_str(), "sdk", OS_NAME, NULL);
		FileUtils::ListDir(sdkPath, paths);

		vector<string>::iterator sdkVersion = paths.begin();
		while (sdkVersion != paths.end())
		{
			string version = *sdkVersion++;
			string fullPath = FileUtils::Join(sdkPath.c_str(), version.c_str(), NULL);
			c = KComponent::NewComponent(SDK, "sdk", version, fullPath);
			AddToComponentVector(results, c);
		}
	}

	void ScanMobileSDKsAtPath(string path, vector<SharedComponent>& results)
	{
		vector<string> paths;
		SharedComponent c;
		if (!FileUtils::IsDirectory(path))
		{
			return;
		}

		// Read everything that looks like <searchpath>/mobilesdk/<os>/*
		string sdkPath = FileUtils::Join(path.c_str(), "sdk", OS_NAME, NULL);
		FileUtils::ListDir(sdkPath, paths);

		vector<string>::iterator sdkVersion = paths.begin();
		while (sdkVersion != paths.end())
		{
			string version = *sdkVersion++;
			string fullPath = FileUtils::Join(sdkPath.c_str(), version.c_str(), NULL);
			c = KComponent::NewComponent(MOBILESDK, "mobilesdk", version, fullPath);
			AddToComponentVector(results, c);
		}
	}

	void ScanModulesAtPath(string path, vector<SharedComponent>& results)
	{
		vector<string> paths;
		vector<string> subpaths;
		SharedComponent c;

		if (!FileUtils::IsDirectory(path))
		{
			return;
		}

		string namesPath = FileUtils::Join(path.c_str(), "modules", OS_NAME, NULL);

		// Read everything that looks like <searchpath>/modules/<os>/*
		FileUtils::ListDir(namesPath, paths);
		vector<string>::iterator moduleName = paths.begin();
		while (moduleName != paths.end())
		{
			string name = *moduleName++;
			string versionsPath = FileUtils::Join(namesPath.c_str(), name.c_str(), NULL);

			// Read everything that looks like <searchpath>/modules/<os>/<name>/*
			FileUtils::ListDir(versionsPath, subpaths);
			vector<string>::iterator moduleVersion = subpaths.begin();
			while (moduleVersion != subpaths.end())
			{
				string version = *moduleVersion++;
				string fullPath = FileUtils::Join(versionsPath.c_str(), version.c_str(), NULL);
				c = KComponent::NewComponent(MODULE, name, version, fullPath);
				AddToComponentVector(results, c);
			}
		}
	}

	void ScanBundledComponents(string path, vector<SharedComponent>& results)
	{
		vector<string> paths;
		SharedComponent c;

		// Find a directory like <appdir>/runtime/
		string rtPath = FileUtils::Join(path.c_str(), "runtime", NULL);
		if (FileUtils::IsDirectory(rtPath))
		{
			c = KComponent::NewComponent(RUNTIME, "runtime", "", rtPath, true);
			results.push_back(c);
		}

		// Find all directories like <appdir>/modules/*
		string modulesPath = FileUtils::Join(path.c_str(), "modules", NULL);
		FileUtils::ListDir(modulesPath, paths);
		for (size_t i = 0; i < paths.size(); i++)
		{
			string name = paths[i];
			string path = FileUtils::Join(modulesPath.c_str(), name.c_str(), NULL);
			c = KComponent::NewComponent(MODULE, name, "", path, true);
			results.push_back(c);
		}
	}

	SharedDependency Dependency::NewDependency(string key, string value)
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
			d->type = RUNTIME;
		else if (key == "sdk")
			d->type = SDK;
		else if (key=="mobilesdk")
			d->type = MOBILESDK;
		else if (key=="app_update")
			d->type = APP_UPDATE;
		else
			d->type = MODULE;
		return d;
	}

	SharedComponent KComponent::NewComponent(
		KComponentType type, string name, string version, string path, bool bundled)
	{
		KComponent* c = new KComponent();
		c->type = type;
		c->name = name;
		c->version = version;
		c->path = path;
		c->bundled = bundled;
		return c;
	}

	vector<pair<string, string> > KComponent::ReadManifest()
	{
		string manifestPath = FileUtils::Join(this->path.c_str(), MANIFEST_FILENAME, NULL);
		return BootUtils::ReadManifestFile(manifestPath);
	}

	int BootUtils::CompareVersions(string one, string two)
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

	bool BootUtils::WeakCompareComponents(SharedComponent one, SharedComponent two)
	{
		return BootUtils::CompareVersions(one->version, two->version) > 0;
	}

	vector<pair<string, string> > BootUtils::ReadManifestFile(std::string path)
	{
		vector<pair<string, string> > manifest;
		if (!FileUtils::IsFile(path))
		{
			return manifest;
		}

		std::ifstream file(path.c_str());
		if (file.bad() || file.fail())
		{
			return manifest;
		}

		while (!file.eof())
		{
			string line;
			std::getline(file, line);
			line = FileUtils::Trim(line);

			size_t pos = line.find(":");
			if (pos == 0 || pos == line.length() - 1)
			{
				continue;
			}
			else
			{
				string key = line.substr(0, pos);
				string value = line.substr(pos + 1, line.length());
				key = FileUtils::Trim(key);
				value = FileUtils::Trim(value);
				manifest.push_back(pair<string, string>(key, value));
			}
		}
		file.close();
		return manifest;
	}
}
