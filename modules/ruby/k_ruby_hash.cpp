/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"

namespace kroll
{

	static bool ShouldTreatKeysAsSymbols(VALUE hash)
	{
		size_t numberOfSymbolKeys = 0;
		size_t numberOfKeys = 0;

		VALUE keys = rb_funcall(hash, rb_intern("keys"), 0);
		for (int i = 0; i < RARRAY_LEN(keys); i++)
		{
			if (TYPE(rb_ary_entry(keys, i)) == T_SYMBOL)
				numberOfSymbolKeys++;

			numberOfKeys++;
		}

		// If the number of keys that are symbols is grater than
		// zero and more than half of the keys are symbols, than
		// we want to treat all new keys as symbols.
		return ((numberOfSymbolKeys * 2) > numberOfKeys);
	}

	KRubyHash::KRubyHash(VALUE hash) :
		KObject("Ruby.KRubyHash"),
		hash(hash),
		object(new KRubyObject(hash))
	{
		rb_gc_register_address(&hash);
	}

	KRubyHash::~KRubyHash()
	{
		rb_gc_unregister_address(&hash);
	}

	KValueRef KRubyHash::Get(const char *name)
	{
		VALUE keyAsSymbol = ID2SYM(rb_intern(name));
		if (rb_funcall(hash, rb_intern("has_key?"), 1, keyAsSymbol))
			return RubyUtils::ToKrollValue(rb_hash_aref(hash, keyAsSymbol));

		VALUE keyAsString = rb_str_new2(name);
		if (rb_funcall(hash, rb_intern("has_key?"), 1, keyAsString))
			return RubyUtils::ToKrollValue(rb_hash_aref(hash, keyAsString));

		return this->object->Get(name);
	}

	void KRubyHash::Set(const char* name, KValueRef value)
	{
		// If this hash already has a key that's a symbol of
		// this name, then just use the symbol version. This
		// allows Kroll to work with hashes of symbols (pretty
		// common in Ruby) without really *knowing* about symbols.
		VALUE keyAsSymbol = ID2SYM(rb_intern(name));
		if (rb_funcall(hash, rb_intern("has_key?"), 1, keyAsSymbol)
			|| ShouldTreatKeysAsSymbols(hash))
		{
			rb_hash_aset(hash, keyAsSymbol,
				RubyUtils::ToRubyValue(value));
		}
		else
		{
			rb_hash_aset(hash, rb_str_new2(name),
				RubyUtils::ToRubyValue(value));
		}
	}

	SharedString KRubyHash::DisplayString(int levels)
	{
		return this->object->DisplayString(levels);
	}

	SharedStringList KRubyHash::GetPropertyNames()
	{
		SharedStringList property_names = object->GetPropertyNames();

		SharedStringList names(this->object->GetPropertyNames());
		VALUE keys = rb_funcall(hash, rb_intern("keys"), 0);
		for (int i = 0; i < RARRAY_LEN(keys); i++)
		{
			VALUE key = rb_ary_entry(keys, i);
			if (TYPE(key) == T_SYMBOL)
				names->push_back(new std::string(rb_id2name(SYM2ID(key))));
			else if (TYPE(key) == T_STRING)
				names->push_back(new std::string(StringValuePtr(key)));
		}

		return names;
	}

	VALUE KRubyHash::ToRuby()
	{
		return this->object->ToRuby();
	}

	bool KRubyHash::Equals(KObjectRef other)
	{
		AutoPtr<KRubyHash> hashOther = other.cast<KRubyHash>();
		if (hashOther.isNull())
			return false;
		return hashOther->ToRuby() == this->ToRuby();
	}
}
