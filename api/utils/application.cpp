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
	namespace BootUtils
	{
		extern void ScanBundledComponents(string, vector<SharedComponent>&);
	}

	/*static*/
	SharedPtr<Application> Application::NewApplication(string appPath)
	{
		string manifest(FileUtils::Join(appPath.c_str(), MANIFEST_FILENAME, NULL));
		return Application::NewApplication(manifest, appPath);
	}

	/*static*/
	SharedApplication Application::NewApplication(vector<pair<string, string> >& manifest)
	{
		Application* application = new Application("", "");
		application->ParseManifest(manifest);
		return application;
	}
	
	/*static*/
	SharedPtr<Application> Application::NewApplication(
		string manifestPath,
		string appPath)
	{
		vector<pair<string, string> > manifest =
			BootUtils::ReadManifestFile(manifestPath);
		if (manifest.empty())
		{
			return NULL;
		}

		Application* application = new Application(appPath, manifestPath);
		application->ParseManifest(manifest);
		return application;
	}
	
	void Application::ParseManifest(vector<pair<string, string> >& manifest)
	{
		vector<pair<string, string> >::iterator i = manifest.begin();
		while (i != manifest.end())
		{
			string key(i->first);
			string value(i->second);
			*i++;

			if (key == "#appname")
			{
				this->name = value;
				continue;
			}
			else if (key == "#appid")
			{
				this->id = value;
				continue;
			}
			else if (key == "#guid")
			{
				this->guid = value;
				continue;
			}
			else if (key == "#image")
			{
				this->image =
					 FileUtils::Join(this->GetResourcesPath().c_str(), value.c_str(), NULL);
				continue;
			}
			else if (key == "#publisher")
			{
				this->publisher = value;
				continue;
			}
			else if (key == "#url")
			{
				this->url = value;
				continue;
			}
			else if (key == "#version")
			{
				this->version = value;
				continue;
			}
			else if (key == "#stream")
			{
				this->stream = value;
				continue;
			}
			else if (key == "#loglevel")
			{
				this->logLevel = value;
				continue;
			}
			else if (key[0] == '#')
			{
				continue;
			}
			else
			{
				SharedDependency d = Dependency::NewDependencyFromManifestLine(key, value);
				this->dependencies.push_back(d);
			}
		}

		if (EnvironmentUtils::Has("TITANIUM_STREAM"))
			this->stream = EnvironmentUtils::Get("TITANIUM_STREAM");
	}

	Application::Application(string path, string manifestPath) :
		path(path),
		manifestPath(manifestPath),
		stream("production")
	{
		// Default stream is production and is optional and doesn't have to be 
		// in the manifest unless switching from production to test or dev
	}

	Application::~Application()
	{
		this->modules.clear();
		this->runtime = NULL;
	}

	string Application::GetExecutablePath()
	{
		// TODO:
		// If this application has arguments, it's probably the currently running
		// application, so we can try to get the executable path based on argv[0]
		string exeName(this->name + ".exe");
		string path(FileUtils::Join(this->path.c_str(), exeName.c_str(), NULL));
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		path = FileUtils::Join(this->path.c_str(), "MacOS", this->name.c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		path = FileUtils::Join(this->path.c_str(), this->name.c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		return string();
	}

	string Application::GetComponentPath(string name)
	{
		std::transform(name.begin(), name.end(), name.begin(), tolower);
		if (name == "runtime")
		{
			return this->runtime->path;
		}
		else
		{
			vector<SharedComponent>::iterator i = this->modules.begin();
			while (i != this->modules.end())
			{
				SharedComponent comp = *i++;
				if (comp->name == name)
				{
					return comp->path;
				}
			}
		}
		return string();
	}

	string Application::GetDataPath()
	{
		return FileUtils::GetApplicationDataDirectory(this->id);
	}

	string Application::GetResourcesPath()
	{
		return FileUtils::Join(this->path.c_str(), "Resources", NULL);
	}

	bool Application::IsInstalled()
	{
		string dataDirMarker(FileUtils::Join(this->GetDataPath().c_str(),
			INSTALLED_MARKER_FILENAME, NULL));
		string appDirMarker(FileUtils::Join(this->path.c_str(),
			INSTALLED_MARKER_FILENAME, NULL));
		return FileUtils::IsFile(dataDirMarker) ||
			FileUtils::IsFile(appDirMarker);
	}

	vector<SharedDependency> Application::ResolveDependencies()
	{
		this->modules.clear(); // Blank slate
		this->sdks.clear();
		this->runtime = NULL;
		vector<SharedComponent> components;
		this->GetAvailableComponents(components);

		vector<SharedDependency> unresolved;
		vector<SharedDependency>::iterator i = this->dependencies.begin();
		while (i != this->dependencies.end())
		{
			SharedDependency d = *i++;
			SharedComponent c = BootUtils::ResolveDependency(d, components);
			if (c.isNull())
			{
				unresolved.push_back(d);
			}
			else if (c->type == MODULE)
			{
				this->modules.push_back(c);
			}
			else if (c->type == SDK)
			{
				this->sdks.push_back(c);
			}
			else if (c->type == MOBILESDK)
			{
				this->sdks.push_back(c);
			}
			else if (c->type == RUNTIME)
			{
				this->runtime = c;
			}
		}

		return unresolved;
	}

	string& Application::GetStreamURL(const char* scheme)
	{
		static string url;
		if (!url.empty())
			return url;

		if (stream == "local" || stream == "l")
		{
			url = "http://localhost";
		}
		else
		{
			url = scheme;
			url.append("://"STRING(_DISTRIBUTION_URL));
			if (stream == "production" || stream == "p")
			{
				url.append("/p");
			}
			else if (stream == "dev" || stream == "d")
			{
				url.append("/d");
			}
			else if (stream == "test" || stream == "t")
			{
				url.append("/t");
			}
			else
			{
				url.append("/");
				url.append(this->stream);
			}
			url.append("/v1");
		}

		return url;
	}

	string Application::GetURLForDependency(SharedDependency d)
	{
		// First look for a bundled zip in the "dist" directory
		string zipfile;
		if (d->type == RUNTIME)
		{
			zipfile = string("runtime-") + d->version + ".zip";
		}
		else if (d->type == SDK)
		{
			zipfile = string("sdk-") +  d->version + ".zip";
		}
		else if (d->type == MOBILESDK)
		{
			zipfile = string("mobilesdk-") +  d->version + ".zip";
		}
		else if (d->type == APP_UPDATE)
		{
			zipfile = "appupdate";
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
			queryString = this->GetStreamURL();
			queryString += "/release-download";

			string mid(PlatformUtils::GetMachineId());
			string os(OS_NAME);
			string osver(FileUtils::GetOSVersion());
			string osarch(FileUtils::GetOSArchitecture());

			this->queryString += "?os=" + URLUtils::EncodeURIComponent(os);
			this->queryString += "&osver=" + URLUtils::EncodeURIComponent(osver);
			this->queryString += "&tiver=" + URLUtils::EncodeURIComponent(PRODUCT_VERSION);
			this->queryString += "&mid=" + URLUtils::EncodeURIComponent(mid);
			this->queryString += "&aid=" + URLUtils::EncodeURIComponent(this->id);
			this->queryString += "&guid=" + URLUtils::EncodeURIComponent(this->guid);
			this->queryString += "&ostype=" + URLUtils::EncodeURIComponent(OS_TYPE);
			this->queryString += "&osarch=" + URLUtils::EncodeURIComponent(osarch);
		}

		string url(this->queryString);
		url.append("&name=");
		url.append(d->name);
		url.append("&version=");
		url.append(d->version);
		url.append("&uuid=");
		if (d->type == RUNTIME)
			url.append(RUNTIME_UUID);
		if (d->type == SDK)
			url.append(SDK_UUID);
		if (d->type == MOBILESDK)
			url.append(MOBILESDK_UUID);
		if (d->type == MODULE)
			url.append(MODULE_UUID);
		if (d->type == APP_UPDATE)
			url.append(APP_UPDATE_UUID);
		return url;
	}

	string Application::GetLicenseText()
	{
		string text;

		string license(FileUtils::Join(this->path.c_str(),
			LICENSE_FILENAME, NULL));
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
		SharedDependency d = Dependency::NewDependencyFromValues(
			APP_UPDATE, "app_update", this->version);
		return GetURLForDependency(d);
	}

	void Application::GetAvailableComponents(vector<SharedComponent>& components, bool onlyBundled)
	{
		if (this->HasArgument(OVERRIDE_ARG))
		{
			// Only scan bundled components on the override path
			string overridePath(this->GetArgumentValue(OVERRIDE_ARG));
			BootUtils::ScanBundledComponents(overridePath, components); 
			onlyBundled = true;
		}
		else
		{
			// Merge bundled and installed components
			BootUtils::ScanBundledComponents(this->path, components); 
		}

		if (!onlyBundled)
		{
			vector<SharedComponent>& installedComponents =
				BootUtils::GetInstalledComponents(true);
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
		string dashNeedle(string("--") + needle);
		vector<string>::iterator i = this->arguments.begin();
		while (i != this->arguments.end())
		{
			string arg(*i++);
			if (arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
			{
				return true;
			}
		}
		return false;
	}

	string Application::GetArgumentValue(string needle)
	{
		string dashNeedle(string("--") + needle);
		vector<string>::iterator i = this->arguments.begin();
		while (i != this->arguments.end())
		{
			string arg(*i++);
			size_t start;
			if ((arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
				 && (start = arg.find("=")) != string::npos)
			{
				string value(arg.substr(start + 1));
				if (value[0] == '"' && value.length() > 3)
				{
					value = value.substr(1, value.length() - 2);
				}
				return value;
			}
		}
		return string();
	}

	vector<SharedComponent> Application::GetResolvedComponents()
	{
		vector<SharedComponent> resolved;

		if (this->runtime)
			resolved.push_back(this->runtime);

		resolved.reserve(resolved.size() + this->modules.size() + this->sdks.size());
		resolved.insert(resolved.end(), this->modules.begin(), this->modules.end());
		resolved.insert(resolved.end(), this->sdks.begin(), this->sdks.end());

		return resolved;
	}
}
