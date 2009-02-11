/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>

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
			for (int i = 0; i < this->Size(); i++)
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

}

