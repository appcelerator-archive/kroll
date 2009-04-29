/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"
#define OVERRIDE_ARG "--bundled-component-override"

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
	extern void ScanRuntimesAtPath(string, vector<SharedComponent>&);
	extern void ScanModulesAtPath(string, vector<SharedComponent>&);
	extern void ScanBundledComponents(string, vector<SharedComponent>&);

	SharedPtr<Application> Application::NewApplication(string appPath)
	{
		string manifest = FileUtils::Join(appPath.c_str(), MANIFEST_FILENAME, NULL);
		return Application::NewApplication(manifest, appPath);
	}

	SharedPtr<Application> Application::NewApplication(string manifest, string appPath)
	{
		if (!FileUtils::IsFile(manifest))
		{
			return NULL;
		}

		std::ifstream file(manifest.c_str());
		if (file.bad() || file.fail())
		{
			return NULL;
		}

		Application* application = new Application();
		application->path = appPath;
		
		// default is production and is optional and doesn't have to be 
		// in the manifest unless switching from production to test or dev
		application->stream = "production"; 

		while (!file.eof())
		{
			string line;
			std::getline(file, line);
			line = FileUtils::Trim(line);

			size_t pos = line.find(":");
			if (pos == 0 || pos == line.length() - 1)
				continue;
			

			string key = line.substr(0, pos);
			string value = line.substr(pos + 1, line.length());
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
			else if (key == "#stream")
			{
				application->stream = value;
				continue;
			}
			else if (key.c_str()[0] == '#')
			{
				continue;
			}
			else
			{
				SharedDependency d = Dependency::NewDependency(key, value);
				application->dependencies.push_back(d);
			}
		}

		file.close();
		return application;
	}

	Application::~Application()
	{
		this->modules.clear();
		this->runtime = NULL;
	}

	bool Application::IsInstalled()
	{
		string marker = FileUtils::Join(
			this->path.c_str(),
			INSTALLED_MARKER_FILENAME,
			NULL);
		return FileUtils::IsFile(marker);
	}

	vector<SharedDependency> Application::ResolveDependencies()
	{
		this->modules.clear(); // Blank slate
		this->runtime = NULL;
		vector<SharedComponent> components;
		this->GetAvailableComponents(components);

		vector<SharedDependency> unresolved;
		vector<SharedDependency>::iterator i = this->dependencies.begin();
		while (i != this->dependencies.end())
		{
			SharedDependency d = *i++;
			SharedComponent c = this->ResolveDependency(d, components);
			if (c.isNull())
			{
				unresolved.push_back(d);
			}
			else if (c->type == MODULE)
			{
				this->modules.push_back(c);
			}
			else if (c->type == RUNTIME)
			{
				this->runtime = c;
			}
		}

		return unresolved;
	}

	string Application::GetURLForDependency(SharedDependency d)
	{
		// First look for a bundled zip in the "dist" directory
		string zipfile;
		if (d->type == RUNTIME)
		{
			zipfile = string("runtime-") + d->version + ".zip";
		}
		else
		{
			zipfile = string("module-") + d->name + "-" + d->version + ".zip";
		}
		zipfile = FileUtils::Join(this->path.c_str(), "dist", zipfile.c_str(), NULL);
		if (FileUtils::IsFile(zipfile))
			return zipfile;

		// Otherwise return a URL on the distribution site
		if (this->queryString.empty()) // Lazy caching of app query string
		{
			this->queryString = DISTRIBUTION_URL;
			if (EnvironmentUtils::Has(BOOT_UPDATESITE_ENVNAME))
				this->queryString = EnvironmentUtils::Get(BOOT_UPDATESITE_ENVNAME);

			string mid = PlatformUtils::GetMachineId();
			string os = OS_NAME;
			string osver = FileUtils::GetOSVersion();
			string osarch = FileUtils::GetOSArchitecture();

			this->queryString += "?os=" + DataUtils::EncodeURIComponent(os);
			this->queryString += "&osver=" + DataUtils::EncodeURIComponent(osver);
			this->queryString += "&tiver=" + DataUtils::EncodeURIComponent(STRING(_PRODUCT_VERSION));
			this->queryString += "&mid=" + DataUtils::EncodeURIComponent(mid);
			this->queryString += "&aid=" + DataUtils::EncodeURIComponent(this->id);
			this->queryString += "&guid=" + DataUtils::EncodeURIComponent(this->guid);
			this->queryString += "&ostype=" + DataUtils::EncodeURIComponent(OS_TYPE);
			this->queryString += "&osarch=" + DataUtils::EncodeURIComponent(osarch);
		}

		string url = this->queryString;
		url.append("&name=");
		url.append(d->name);
		url.append("&version=");
		url.append(d->version);
		url.append("&uuid=");
		if (d->type == RUNTIME)
			url.append(RUNTIME_UUID);
		if (d->type == MODULE)
			url.append(MODULE_UUID);
		return url;
	}

	string Application::GetLicenseText()
	{
		string text;

		string license = FileUtils::Join(this->path.c_str(), LICENSE_FILENAME, NULL);
		if (!FileUtils::IsFile(license))
			return text;

		std::ifstream file(license.c_str());
		if (file.bad() || file.fail())
			return text;

		while (!file.eof())
		{
			string line;
			std::getline(file, line);
			text.append(line);
			text.append("\n");
		}
		return text;
	}

	string Application::GetUpdateURL()
	{
		return "nourlyet";
	}

	SharedComponent Application::ResolveDependency(SharedDependency dep, vector<SharedComponent>& components)
	{
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent comp = *i++;
			if (dep->type != comp->type || dep->name != comp->name)
				continue;

			// Always give preference to bundled components, otherwise do a normal comparison
			int compare = BootUtils::CompareVersions(comp->version, dep->version);
			if (comp->bundled
				|| (dep->requirement == Dependency::EQ && compare == 0)
				|| (dep->requirement == Dependency::GTE && compare >= 0)
				|| (dep->requirement == Dependency::GT && compare > 0)
				|| (dep->requirement == Dependency::LT && compare < 0))
			{
				return comp;
			}
		}

		return NULL;
	} 

	void Application::GetAvailableComponents(vector<SharedComponent>& components)
	{
		if (this->HasArgument(OVERRIDE_ARG))
		{
			// Only scan bundled components on the override path
			string overridePath = this->GetArgumentValue(OVERRIDE_ARG);
			ScanBundledComponents(overridePath, components); 
		}
		else
		{
			// Merge bundled and installed components
			ScanBundledComponents(this->path, components); 
			vector<SharedComponent>& installedComponents = BootUtils::GetInstalledComponents(true);
			for (size_t i = 0; i < installedComponents.size(); i++)
			{
				components.push_back(installedComponents.at(i));
			}
		}
	}

	void Application::UsingModule(string name, string version, string path)
	{
		// Ensure that this module is not already in our list of modules.
		vector<SharedComponent>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			SharedComponent c = *i++;
			if (c->name == name)
			{
				// Bundled modules currently do not know their version until
				// they are loaded, so update the version field of the component.
				c->version = version;
				return;
			}
		}

		// It's not in the list so add it.
		SharedComponent c = KComponent::NewComponent(MODULE, name, version, path);
		this->modules.push_back(c);
	}

	void Application::SetArguments(int argc, const char* argv[])
	{
		for (int i = 0; i < argc; i++)
		{
			this->arguments.push_back(argv[i]);
		}
	}

	void Application::SetArguments(vector<string>& arguments)
	{
		this->arguments = arguments;
	}

	vector<string>& Application::GetArguments()
	{
		return this->arguments;
	}

	bool Application::HasArgument(string needle)
	{
		string dashNeedle = string("--") + needle;
		vector<string>::iterator i = this->arguments.begin();
		while (i != this->arguments.end())
		{
			string arg = *i++;
			if (arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
			{
				return true;
			}
		}
		return false;
	}

	string Application::GetArgumentValue(string needle)
	{
		string dashNeedle = string("--") + needle;
		vector<string>::iterator i = this->arguments.begin();
		while (i != this->arguments.end())
		{
			string arg = *i++;
			size_t start;
			if ((arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
				 && (start = arg.find("=")) != string::npos)
			{
				string value = arg.substr(start + 1);
				if (value[0] == '"' && value.length() > 3)
				{
					value = value.substr(1, value.length() - 2);
				}
				return value;
			}
		}
		return string();
	}
}
