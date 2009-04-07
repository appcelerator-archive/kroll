/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include "profiled_bound_list.h"
#include <cstdio>
#include <cstring>

namespace kroll
{
	ProfiledBoundList::ProfiledBoundList(std::string name, SharedKList delegate, Poco::FileOutputStream *stream) : ProfiledBoundObject(name,delegate,stream), list(delegate)
	{
	}

	ProfiledBoundList::~ProfiledBoundList()
	{
	}

	/**
	 * Append a value to this list
	 *  Errors will result in a thrown ValueException
	 */
	void ProfiledBoundList::Append(SharedValue value)
	{
		list->Append(value);
	}

	/**
	 * Get the length of this list.
	 */
	unsigned int ProfiledBoundList::Size()
	{
		return list->Size();
	}

	/**
	 * @return the value at the given index.
	 * Errors will result in a thrown ValueException
	 */
	SharedValue ProfiledBoundList::At(unsigned int index)
	{
		return list->At(index);
	}

	/**
	 * Set the value at the given index. If the index is greater
	 * than the current list length, the list will be lengenthed
	 * by appending Value::Undefined;
	 * Errors will result in a thrown ValueException
	 */
	void ProfiledBoundList::SetAt(unsigned int index, SharedValue value)
	{
		list->SetAt(index,value);
	}

	/**
	 * Remove the list entry at the given index. Return true
	 * if found and removed.
	 * Errors will result in a thrown ValueException
	 */
	bool ProfiledBoundList::Remove(unsigned int index)
	{
		return list->Remove(index);
	}

	/**
	 * Set a property on this object to the given value
	 * Errors will result in a thrown ValueException
	 */
	void ProfiledBoundList::Set(const char *name, SharedValue value)
	{
		list->Set(name,value);
	}

	/**
	 * @return the property with the given name or Value::Undefined
	 * if the property is not found.
	 * Errors will result in a thrown ValueException
	 */
	SharedValue ProfiledBoundList::Get(const char *name)
	{
		return list->Get(name);
	}

	/**
	 * @return a list of this object's property names.
	 */
	SharedStringList ProfiledBoundList::GetPropertyNames()
	{
		return list->GetPropertyNames();
	}
}
