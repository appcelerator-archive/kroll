/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"
#include <cstring>

namespace kroll
{
	KRubyList::KRubyList(VALUE list) :
		list(list),
		object(new KRubyObject(list))
	{
		rb_gc_register_address(&list);
	}

	KRubyList::~KRubyList()
	{
		rb_gc_unregister_address(&list);
	}

	void KRubyList::Append(SharedValue value)
	{
		VALUE rv = RubyUtils::ToRubyValue(value);
		rb_ary_push(list, rv);
	}

	unsigned int KRubyList::Size()
	{
		return (unsigned int) RARRAY(list)->len;
	}

	bool KRubyList::Remove(unsigned int index)
	{
		VALUE v = rb_ary_delete_at(list, index);
		return (v != Qnil);
	}

	SharedValue KRubyList::At(unsigned int index)
	{
		if (index >= 0 && index < this->Size())
		{
			VALUE v = rb_ary_entry(list, index);
			return RubyUtils::ToKrollValue(v);
		}
		else
		{
			return Value::Undefined;
		}
	}

	void KRubyList::Set(const char *name, SharedValue value)
	{
		// Check for integer value as name
		if (KList::IsInt(name))
		{
			int index = atoi(name);
			if (index >= 0)
				return this->SetAt(index, name);
		}

		this->object->Set(name, value);
	}

	void SetAt(unsigned int index, SharedValue value)
	{
		while (index >= this->Size())
		{
			// now we need to create entries between current size
			//  and new size and make the entries undefined.
			this->Append(Value::Undefined);
		}
		VALUE rv = RubyUtils::ToRubyValue(value);
		rb_ary_store(list, index, rv);
	}

	SharedValue KRubyList::Get(const char *name)
	{
		if (strcmp("length", name) == 0)
		{
			return Value::NewInt(this->Size());
		}

		if (KList::IsInt(name))
		{
			unsigned int index = (unsigned int) atoi(name);
			if (index >= 0)
				return this->At(index);
		}

		return object->Get(name);
	}

	SharedStringList KRubyList::GetPropertyNames()
	{
		SharedStringList property_names = object->GetPropertyNames();
		property_names->push_back(new std::string("length"));
		for (size_t i = 0; i < this->Size(); i++)
		{
			std::string name = KList::IntToChars(i);
			property_names->push_back(new std::string(name));
		}

		return property_names;
	}

	VALUE KRubyList::ToRuby()
	{
		return this->object->ToRuby();
	}

	SharedString KRubyList::DisplayString(int levels)
	{
		return this->object->DisplayString(levels);
	}

}
