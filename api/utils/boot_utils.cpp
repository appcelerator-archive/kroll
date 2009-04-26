/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

using std::string;
using std::vector;
using std::pair;

namespace kroll
{

	string BootUtils::FindBundledModuleZip(
		string name,
		string version,
		string applicationDirectory)
	{
		string zipName;
		if (name != "runtime")
			zipName.append("module-");
		zipName.append(name);
		zipName.append("-");
		zipName.append(version);
		zipName.append(".zip");

		string zipLocation = FileUtils::Join(
			applicationDirectory.c_str(),
			"dist",
			zipName.c_str(),
			NULL);

		if (FileUtils::IsFile(zipLocation))
			return zipLocation;
		else
			return string();
	}
	
	std::string BootUtils::FindCommandLineArg(std::string name, std::string def, int argc, char *argv[])
	{
		for (int c=1;c<argc;c++)
		{
			std::string arg = std::string(argv[c]);
			std::string s = "--" + name + "=";
			if (arg.find(s)==0)
			{
				std::size_t pos = arg.find("=");
				std::string v = arg.substr(pos+1);
				if (v[0]=='"')
				{
					// trim off quotes
					return v.substr(1,v.length()-2);
				}
				return v;
			}
		}
		return def;
	}

	Application* BootUtils::ReadManifest(string appPath,int argc, char *argv[])
	{
		string manifest = FileUtils::Join(appPath.c_str(), MANIFEST_FILENAME, NULL);
		return ReadManifestFile(manifest, appPath, argc, argv);
	}

	Application* BootUtils::ReadManifestFile(string manifest, string appPath, int argc, char *argv[])
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

		// For now this application uses the system runtime home, but during module
		// resolution, it will use whatever runtime home it finds the runtime in.
		application->runtimeHomePath = FileUtils::GetSystemRuntimeHomeDirectory();

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
			else if (key.c_str()[0] == '#')
			{
				continue;
			}
			else
			{
				KComponent* c = new KComponent(key, value);
				if (c->typeGuid == RUNTIME_UUID)
				{
					application->runtime = c;
				}
				else
				{
					application->modules.push_back(c);
				}
			}
		}

		// if we pass in --runtime_override we'll use that as our runtime dir
		std::string rto = FindCommandLineArg("runtime_override","",argc,argv);
		if (!rto.empty())
		{
			application->runtime_override = rto;
#ifdef DEBUG
			std::cout << "Runtime override is = " << rto << std::endl;
#endif
		}

		// if we pass in --module_override we'll use that as our module dir
		std::string mo = FindCommandLineArg("module_override","",argc,argv);
		if (!mo.empty())
		{
			application->module_override = mo;
#ifdef DEBUG
			std::cout << "Module override is = " << mo << std::endl;
#endif
		}
		
		file.close();
		return application;
	}

	bool BootUtils::WeakCompareVersions(string one, string two)
	{
		return CompareVersions(one, two) > 0;
	}

	int BootUtils::CompareVersions(string one, string two)
	{
		if (one.empty() && two.empty())
			return 0;
		if (one.empty())
			return -1;
		if (two.empty())
			return 1;

		string delim = ".";
		vector<string> listOne;
		vector<string> listTwo;
		FileUtils::Tokenize(one, listOne, delim);
		FileUtils::Tokenize(two, listTwo, delim);

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

	pair<Requirement, string> KComponent::ParseVersion(string& specStr)
	{
		pair<Requirement, string> spec;

		if (specStr.find(">=") != string::npos)
		{
			spec.first = GTE;
			spec.second = specStr.substr(2, specStr.length());
		}
		else if (specStr.find("<=") != string::npos)
		{
			spec.first = LTE;
			spec.second = specStr.substr(2, specStr.length());
		}
		else if (specStr.find("<") != string::npos)
		{
			spec.first = LT;
			spec.second = specStr.substr(1, specStr.length());
		}
		else if (specStr.find(">") != string::npos)
		{
			spec.first = GT;
			spec.second = specStr.substr(1, specStr.length());
		}
		else if (specStr.find("=") != string::npos)
		{
			spec.first = EQ;
			spec.second = specStr.substr(1, specStr.length());
		}
		else
		{
			spec.first = EQ;
			spec.second = specStr;
		}
		return spec;
	}

	KComponent::KComponent(string key, string value)
	{
		pair<Requirement, string> spec = ParseVersion(value);
		this->requirement = spec.first;
		this->version = spec.second;
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

	string BootUtils::FindVersionedSubfolder(
		string path,
		Requirement req,
		string version)
	{
		vector<string> found;
		vector<string> files;
		FileUtils::ListDir(path, files);
		vector<string>::iterator iter = files.begin();
		while (iter != files.end())
		{
			string str = (*iter++);
			string fullpath = FileUtils::Join(path.c_str(), str.c_str(), NULL);
			if (str != "." && str == ".." && FileUtils::IsDirectory(fullpath))
				continue;

			int compare = CompareVersions(str, version);
			if (req == EQ && compare == 0)
			{
				found.push_back(str);
			}
			else if (req == GTE && compare >= 0)
			{
				found.push_back(str);
			}
			else if (req == LTE && compare <= 0)
			{
				found.push_back(str);
			}
			else if (req == GT && compare > 0)
			{
				found.push_back(str);
			}
			else if (req == LT && compare < 0)
			{
				found.push_back(str);
			}
			
		}

		if (found.size() > 0)
		{
			std::sort(found.begin(), found.end(), WeakCompareVersions);
			string file = found[0];
			return FileUtils::Join(path.c_str(), file.c_str(), NULL);
		}
		else
		{
			return string();
		}
	}

	bool KComponent::Resolve(Application* app, vector<string>& runtimeHomes)
	{
		string path = app->path.c_str();
		
		// see if we have command line overrides (priority)
		if (this->typeGuid == MODULE_UUID && !app->module_override.empty())
		{
			path = FileUtils::Join(app->module_override.c_str(), this->name.c_str(), NULL);
		}
		else if (this->typeGuid == RUNTIME_UUID && !app->runtime_override.empty())
		{
			path = app->runtime_override;
		}
		else
		{
			// Try to find the bundled version of this module.
			if (this->typeGuid == MODULE_UUID)
			{
				path = FileUtils::Join(path.c_str(), "modules", this->name.c_str(), NULL);
			}
			else if (this->typeGuid == RUNTIME_UUID)
			{
				path = FileUtils::Join(path.c_str(), "runtime", NULL);
			}
			else
			{
				return false;
			}
		}
		
		if (FileUtils::IsDirectory(path))
		{
			this->path = path;
			return true;
		}

		vector<string>::iterator i = runtimeHomes.begin();
		while (i != runtimeHomes.end())
		{
			string rth = *i++;
			if (this->typeGuid == MODULE_UUID)
			{
				path = FileUtils::Join(rth.c_str(), "modules", OS_NAME, this->name.c_str(), NULL);
			}
			else
			{
				path = FileUtils::Join(rth.c_str(), "runtime", OS_NAME, NULL);
			}

			string result = BootUtils::FindVersionedSubfolder(
				path, this->requirement, this->version);
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

	string KComponent::GetURL(Application* app)
	{
		string url = app->GetQueryString();
		url.append("&name=");
		url.append(this->name);
		url.append("&version=");
		url.append(this->version);
		url.append("&uuid=");
		url.append(this->typeGuid);
		return url;
	}

	vector<KComponent*> Application::ResolveAllComponents(vector<string>& runtimeHomes)
	{
		vector<KComponent*> unresolved;

		if (this->runtime != NULL && !this->runtime->Resolve(this, runtimeHomes))
		{
			unresolved.push_back(this->runtime);
		}

		// Find all regular modules
		vector<KComponent*>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			KComponent* m = *i++;
#ifdef DEBUG
			printf("resolving: %s\n", m->name.c_str());
#endif
			if (!m->Resolve(this, runtimeHomes))
			{
				unresolved.push_back(m);
			}
		}

		return unresolved;
	}

	bool Application::IsInstalled()
	{
		string marker = FileUtils::Join(
			this->path.c_str(),
			INSTALLED_MARKER_FILENAME,
			NULL);
		return FileUtils::IsFile(marker);
	}

	string Application::GetQueryString()
	{
		if (this->queryString.empty())
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
		return this->queryString;
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

	Application::~Application()
	{
		vector<KComponent*>::iterator i = modules.begin();
		while (i != modules.end())
		{
			KComponent* c = *i;
			i = modules.erase(i);
			delete c;
		}

		delete this->runtime;
	}
	
}
