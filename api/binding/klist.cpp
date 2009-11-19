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

	void KList::ResizeTo(unsigned int size)
	{
		if (size < 0 || size == this->Size())
		{
			return;
		}

		while (size < this->Size())
		{
			this->Remove(this->Size() - 1);
		}

		while (size > this->Size())
		{
			this->Append(Value::Undefined);
		}
	}

	SharedString KList::DisplayString(int levels)
	{
		std::ostringstream oss;
		oss << "(" << this->GetType() << ")" << " [";
		for (unsigned int i = 0; i < this->Size(); i++)
		{
			KValueRef list_val = this->At(i);
			SharedString list_str = list_val->DisplayString(levels-1);
			oss << " " << *list_str << ",";
		}
		//int before_last_comma = oss.tellp() - 1;
		//oss.seekp(before_last_comma);
		oss << " ]";

		return new std::string(oss.str());
	}

	std::string KList::IntToChars(unsigned int value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	bool KList::IsInt(const std::string& name)
	{
		for (size_t i = 0; i < strlen(name.c_str()); i++)
		{
			if (!isdigit(name[i]))
				return false;
		}
		return true;
	}

	KListRef KList::Unwrap(KListRef o)
	{
		AutoPtr<ProfiledBoundList> plist = o.cast<ProfiledBoundList>();
		if (plist.isNull())
		{
			return o;
		}
		else
		{
			return plist->GetDelegate();
		}
	}
}

