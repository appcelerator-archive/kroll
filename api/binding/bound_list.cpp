/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>
#include <cmath>

namespace kroll
{

	SharedString BoundList::DisplayString(int levels)
	{
		std::ostringstream oss;
		if (levels == 0)
		{
			oss << "<BoundList at " << this << ">";
		}
		else
		{

			oss << "[";
			for (unsigned int i = 0; i < this->Size(); i++)
			{
				SharedValue list_val = this->At(i);
				SharedString list_str = list_val->DisplayString(levels-1);
				oss << " " << *list_str << ",";
			}
			//int before_last_comma = oss.tellp() - 1;
			//oss.seekp(before_last_comma);
			oss << " ]";
		}
		return new std::string(oss.str());
	}

	char* BoundList::IntToChars(unsigned int value)
	{
		int digits = 1;
		if (value > 0)
			digits += floor(log10((double) value));

		char *buf = new char[digits + 1];
		sprintf(buf, "%d", value);
		return buf;
	}

	bool BoundList::IsInt(const char* name)
	{
		for (size_t i = 0; i < strlen(name); i++)
		{
			if (!isdigit(name[i]))
				return false;
		}
		return true;
	}



}

