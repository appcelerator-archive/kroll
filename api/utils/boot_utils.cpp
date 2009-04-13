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
		if (!FileUtils::IsFile(manifest))
			return NULL;

		std::ifstream file(manifest.c_str());
		if (file.bad() || file.fail())
			return NULL;

		Application* application = new Application();
		application->path = appPath;

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
			else if (key.c_str()[0] == '#')
			{
				continue;
			}
			else
			{
				Component* c = new Component(key, value);
				if (c->typeGuid == RUNTIME_UUID)
					application->runtime = c;
				else
					application->modules.push_back(c);
			}
		}

		return application;
	}

	Component::Component(std::string key, std::string value)
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

	std::string Component::GetURL(Application* app)
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

	Application::~Application()
	{
		std::vector<Component*>::iterator i = modules.begin();
		while (i != modules.end())
		{
			Component* c = *i;
			i = modules.erase(i);
			delete c;
		}

		delete this->runtime;
	}
	
}
