/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"

namespace kroll {

	KRubyHash::KRubyHash(VALUE hash) :
		hash(hash),
		object(new KRubyObject(hash))
	{
		rb_gc_register_address(&hash);
	}

	KRubyHash::~KRubyHash()
	{
		rb_gc_unregister_address(&hash);
	}

	SharedValue KRubyHash::Get(const char *name)
	{
		VALUE key = rb_str_new2(name);
		if (rb_funcall(hash, rb_intern("has_key?"), 1, key))
		{
			VALUE v = rb_hash_aref(hash, key);
			return RubyUtils::ToKrollValue(v);
		}
		else
		{
			return this->object->Get(name);
		}
	}

	void KRubyHash::Set(const char *name, SharedValue value)
	{
		VALUE key = rb_str_new2(name);
		VALUE v = RubyUtils::ToRubyValue(value);
		rb_hash_aset(hash, key, v);
	}

	SharedString KRubyHash::DisplayString(int levels)
	{
		return this->object->DisplayString(levels);
	}

	SharedStringList KRubyHash::GetPropertyNames()
	{
		SharedStringList property_names = object->GetPropertyNames();

		SharedStringList names(new StringList());
		VALUE keys = rb_funcall(hash, rb_intern("keys"), 0);
		for (int i = 0; i < RARRAY(keys)->len; i++)
		{
			VALUE key = rb_ary_entry(keys, i);
			std::string name = RubyUtils::ToString(key);
			names->push_back(new std::string(name));
		}

		return names;
	}

	VALUE KRubyHash::ToRuby()
	{
		return this->object->ToRuby();
	}
}
