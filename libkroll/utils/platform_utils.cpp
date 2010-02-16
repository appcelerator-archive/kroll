/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "utils.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#define MID_PREFIX "v2:"

namespace UTILS_NS
{
namespace PlatformUtils
{
	/**
	 * Create a machine ID for this machine and write it to the .PRODUCT_NAME file
	 * in the user runtime home directory. The machine id is specific to a particular
	 * user on a particular OS install and does not uniquely identify a machine.
	 */
	static std::string CreateMachineId(std::string& midFileName);
	static std::string GetOldMachineId(std::string& midFileName);
	static std::string ReadMIDFromFile(std::string& path);

	std::string GetFirstMACAddress()
	{
		MACAddress address;
		memset(&address, 0, sizeof(&address));

		try
		{
			PlatformUtils::GetFirstMACAddressImpl(address);
		}
		catch(...)
		{
			// Just return zeros.
		}

		char result[18];
		std::sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x",
			address[0], address[1], address[2], address[3],
			address[4], address[5]);
		return std::string(result);
	}

	std::string GetMachineId()
	{

		std::string midFileName(std::string(".") + PRODUCT_NAME);
		std::transform(midFileName.begin(), midFileName.end(), midFileName.begin(), tolower);

		std::string midFilePath(FileUtils::GetUserRuntimeHomeDirectory());
		midFilePath = FileUtils::Join(midFilePath.c_str(), midFileName.c_str(), NULL);
		std::string mid(ReadMIDFromFile(midFilePath));

		// If we couldn't read the MID or this is an old MID,
		// create a new one and return it.
		if (mid.empty() || mid.find(MID_PREFIX) != 0)
		{
			mid = CreateMachineId(midFileName);
		}

		return mid;
	}

	static std::string CreateMachineId(std::string& midFileName)
	{
		std::string newMID(MID_PREFIX);
		newMID.append(DataUtils::GenerateUUID());
		newMID.append("|");
		newMID.append(GetOldMachineId(midFileName));
		newMID.append("\n");

		std::string midFilePath(FileUtils::GetUserRuntimeHomeDirectory());
		midFilePath = FileUtils::Join(midFilePath.c_str(), midFileName.c_str(), NULL);
		FileUtils::WriteFile(midFilePath, newMID);

		return newMID;
	}

	static std::string GetOldMachineId(std::string& midFileName)
	{
		// Search for an old MID stored in a file
		std::vector<std::string> possibleMIDFiles;
		std::string path;
		if (EnvironmentUtils::Has("KR_RUNTIME"))
		{
			path = EnvironmentUtils::Get("KR_RUNTIME");
			path = FileUtils::Join(path.c_str(), "..", "..", "..", midFileName.c_str(), NULL);
			possibleMIDFiles.push_back(path);
		}

		path = FileUtils::GetUserRuntimeHomeDirectory();
		path = FileUtils::Join(path.c_str(), midFileName.c_str(), NULL);
		possibleMIDFiles.push_back(path);

		path = FileUtils::GetSystemRuntimeHomeDirectory();
		path = FileUtils::Join(path.c_str(), midFileName.c_str(), NULL);
		possibleMIDFiles.push_back(path);

		std::string mid;
		for (size_t i = 0; i < possibleMIDFiles.size(); i++)
		{
			std::string& currentPath = possibleMIDFiles[i];
			mid = ReadMIDFromFile(currentPath);
			if (!mid.empty())
				return mid;
		}

		// Alternatively hash the MAC address and use that as the old MID
		std::string MACAddress(PlatformUtils::GetFirstMACAddress());
		return DataUtils::HexMD5(MACAddress);
	}

	static std::string ReadMIDFromFile(std::string& path)
	{
		if (!FileUtils::IsFile(path))
			return std::string();

		return FileUtils::Trim(FileUtils::ReadFile(path));
	}
}
}
