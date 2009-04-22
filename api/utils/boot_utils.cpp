/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

namespace kroll
{

	std::string BootUtils::FindBundledModuleZip(
		std::string name,
		std::string version,
		std::string applicationDirectory)
	{
		std::string zipName;
		if (name != "runtime")
			zipName.append("module-");
		zipName.append(name);
		zipName.append("-");
		zipName.append(version);
		zipName.append(".zip");

		std::string zipLocation = FileUtils::Join(
			applicationDirectory.c_str(),
			"dist",
			zipName.c_str(),
			NULL);

		if (FileUtils::IsFile(zipLocation))
			return zipLocation;
		else
			return std::string();
	}

	Application* BootUtils::ReadManifest(std::string appPath)
	{
		std::string manifest = FileUtils::Join(appPath.c_str(), MANIFEST_FILENAME, NULL);
		return ReadManifestFile(manifest, appPath);
	}

	Application* BootUtils::ReadManifestFile(std::string manifest, std::string appPath)
	{
		if (!FileUtils::IsFile(manifest))
			return NULL;

		std::ifstream file(manifest.c_str());
		if (file.bad() || file.fail())
			return NULL;

		Application* application = new Application();
		application->path = appPath;

		// For now this application uses the user runtime home, but during module
		// resolution, it will use whatever runtime home it finds the runtime in.
		application->runtimeHomePath = FileUtils::GetUserRuntimeHomeDirectory();

		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			line = FileUtils::Trim(line);

			size_t pos = line.find(":");
			if (pos == 0 || pos == line.length() - 1)
				continue;

			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1, line.length());
			key = FileUtils::Trim(key);
			value = FileUtils::Trim(value);

			if (key == "#appname")
			{
				application->name = value;
				continue;
			}
			else if (key == "#appid")
			{
				application->id = value;
				continue;
			}
			else if (key == "#guid")
			{
				application->guid = value;
				continue;
			}
			else if (key == "#image")
			{
				application->image = FileUtils::Join(appPath.c_str(), "Resources", value.c_str(), NULL);
				continue;
			}
			else if (key == "#publisher")
			{
				application->publisher = value;
				continue;
			}
			else if (key == "#url")
			{
				application->url = value;
				continue;
			}
			else if (key == "#version")
			{
				application->version = value;
				continue;
			}
			else if (key.c_str()[0] == '#')
			{
				continue;
			}
			else
			{
				KComponent* c = new KComponent(key, value);
				if (c->typeGuid == RUNTIME_UUID)
					application->runtime = c;
				else
					application->modules.push_back(c);
			}
		}

		file.close();
		return application;
	}

	int BootUtils::CompareVersions(std::string one, std::string two)
	{
		if (one.empty() && two.empty())
			return 0;
		if (one.empty())
			return -1;
		if (two.empty())
			return 1;

		std::string delim = ".";
		std::vector<std::string> listOne;
		std::vector<std::string> listTwo;
		FileUtils::Tokenize(one, listOne, delim);
		FileUtils::Tokenize(two, listTwo, delim);

		int min = listOne.size();
		if (listTwo.size() < listOne.size())
			min = listTwo.size();

		for (int i = 0; i < min; i++)
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

	KComponent::KComponent(std::string key, std::string value)
	{
		FileUtils::ExtractVersion(value, &this->requirement, this->version);
		this->name = key;
		if (key == "runtime")
		{
			this->type = "runtime";
			this->typeGuid = RUNTIME_UUID;
		}
		else
		{
			this->type = "module";
			this->typeGuid = MODULE_UUID;
		}
	}

	bool KComponent::Resolve(Application* app, std::vector<std::string>& runtimeHomes)
	{
		// Try to find the bundled version of this module.
		std::string path;
		if (this->typeGuid == MODULE_UUID)
			path = FileUtils::Join(app->path.c_str(), "modules", this->name.c_str(), NULL);
		else
			path = FileUtils::Join(app->path.c_str(), "runtime", NULL);

		if (FileUtils::IsDirectory(path))
		{
			this->path = path;
			return true;
		}

		std::vector<std::string>::iterator i = runtimeHomes.begin();
		while (i != runtimeHomes.end())
		{
			std::string rth = *i++;
			if (this->typeGuid == MODULE_UUID)
				path = FileUtils::Join(rth.c_str(), "modules", OS_NAME, this->name.c_str(), NULL);
			else
				path = FileUtils::Join(rth.c_str(), "runtime", OS_NAME, NULL);

			std::string result = FileUtils::FindVersioned(path, this->requirement, this->version);
			if (!result.empty())
			{
				this->path = result;

				// Mark this path as the application's real runtime home
				if (this->typeGuid == RUNTIME_UUID)
				{
					app->runtimeHomePath = rth;
				}

				return true;
			}
		}

		return false;
	}

	std::string KComponent::GetURL(Application* app)
	{
		std::string url = app->GetQueryString();
		url.append("&name=");
		url.append(this->name);
		url.append("&version=");
		url.append(this->version);
		url.append("&uuid=");
		url.append(this->typeGuid);
		return url;
	}

	std::vector<KComponent*> Application::ResolveAllComponents(std::vector<std::string>& runtimeHomes)
	{
		std::vector<KComponent*> unresolved;

		if (this->runtime != NULL && !this->runtime->Resolve(this, runtimeHomes))
		{
			unresolved.push_back(this->runtime);
		}

		// Find all regular modules
		std::vector<KComponent*>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			KComponent* m = *i++;
			printf("resolving: %s\n", m->name.c_str());
			if (!m->Resolve(this, runtimeHomes))
				unresolved.push_back(m);
		}

		return unresolved;
	}

	bool Application::IsInstalled()
	{
		std::string marker = FileUtils::Join(
			this->path.c_str(),
			INSTALLED_MARKER_FILENAME,
			NULL);
		return FileUtils::IsFile(marker);
	}

	std::string Application::GetQueryString()
	{
		if (this->queryString.empty())
		{
			this->queryString = DISTRIBUTION_URL;
			if (EnvironmentUtils::Has(BOOT_UPDATESITE_ENVNAME))
				this->queryString = EnvironmentUtils::Get(BOOT_UPDATESITE_ENVNAME);

			std::string mid = PlatformUtils::GetMachineId();
			std::string os = OS_NAME;
			std::string osver = FileUtils::GetOSVersion();
			std::string osarch = FileUtils::GetOSArchitecture();

			this->queryString += "?os=" + FileUtils::EncodeURIComponent(os);
			this->queryString += "&osver=" + FileUtils::EncodeURIComponent(osver);
			this->queryString += "&tiver=" + FileUtils::EncodeURIComponent(STRING(_PRODUCT_VERSION));
			this->queryString += "&mid=" + FileUtils::EncodeURIComponent(mid);
			this->queryString += "&aid=" + FileUtils::EncodeURIComponent(this->id);
			this->queryString += "&guid=" + FileUtils::EncodeURIComponent(this->guid);
			this->queryString += "&ostype=" + FileUtils::EncodeURIComponent(OS_TYPE);
			this->queryString += "&osarch=" + FileUtils::EncodeURIComponent(osarch);
		}
		return this->queryString;
	}

	std::string Application::GetLicenseText()
	{
		std::string text;

		std::string license = FileUtils::Join(this->path.c_str(), LICENSE_FILENAME, NULL);
		if (!FileUtils::IsFile(license))
			return text;

		std::ifstream file(license.c_str());
		if (file.bad() || file.fail())
			return text;

		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			text.append(line);
			text.append("\n");
		}
		return text;
	}

	std::string Application::GetUpdateURL()
	{
		return "nourlyet";
	}

	Application::~Application()
	{
		std::vector<KComponent*>::iterator i = modules.begin();
		while (i != modules.end())
		{
			KComponent* c = *i;
			i = modules.erase(i);
			delete c;
		}

		delete this->runtime;
	}
	
}
