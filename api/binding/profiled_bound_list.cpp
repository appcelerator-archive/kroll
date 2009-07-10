/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>

namespace kroll
{
	ProfiledBoundList::ProfiledBoundList(SharedKList delegate) :
		ProfiledBoundObject(delegate),
		list(delegate),
		referenceCount(1)
	{
	}

	ProfiledBoundList::~ProfiledBoundList()
	{
	}

	void ProfiledBoundList::Append(SharedValue value)
	{
		list->Append(value);
	}

	unsigned int ProfiledBoundList::Size()
	{
		return list->Size();
	}

	SharedValue ProfiledBoundList::At(unsigned int index)
	{
		return list->At(index);
	}

	void ProfiledBoundList::SetAt(unsigned int index, SharedValue value)
	{
		list->SetAt(index,value);
	}

	bool ProfiledBoundList::Remove(unsigned int index)
	{
		return list->Remove(index);
	}

	void ProfiledBoundList::Set(const char *name, SharedValue value)
	{
		list->Set(name, value);
	}

	SharedValue ProfiledBoundList::Get(const char *name)
	{
		return list->Get(name);
	}

	SharedStringList ProfiledBoundList::GetPropertyNames()
	{
		return list->GetPropertyNames();
	}

	bool ProfiledBoundList::HasProperty(const char* name)
	{
		return list->HasProperty(name);
	}
}
