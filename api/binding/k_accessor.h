/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_K_ACCESSOR_H_
#define _KR_K_ACCESSOR_H_

namespace kroll
{
	class KAccessor
	{
	protected:
		KAccessor() {}

		inline void MapAccessor(const std::string& originalName, SharedValue value)
		{
			std::string name(originalName);

			if (value->IsMethod())
			{
				size_t location = 0;
				if (name.find("set") == 0 || name.find("get") == 0)
					location = 3;
				else if (name.find("is") == 0)
					location = 2;

				if (location != 0)
				{
					name = name.substr(location);
					std::transform(name.begin(), name.end(), name.begin(), tolower);
					methodMap[name] = originalName;
				}
			}
		}

		inline std::string& FindAccessorName(std::string name)
		{
			static std::string empty;
			std::transform(name.begin(), name.end(), name.begin(), tolower);

			std::map<std::string, std::string>::iterator i = methodMap.find(name);
			if (i == methodMap.end())
				return empty;
			else
				return i->second;
		}

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KAccessor);
		std::map<std::string, std::string> methodMap;
	};
}

#endif
