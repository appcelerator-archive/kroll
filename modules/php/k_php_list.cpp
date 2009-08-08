/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_php_list.h"

namespace kroll
{
	KPhpList::KPhpList(zval *list) :
		list(list)
	{
		if (Z_TYPE_P(list) != IS_ARRAY)
			throw ValueException::FromString("Invalid zval passed. Should be an array type.");
	}

	KPhpList::~KPhpList()
	{
	}

	SharedValue KPhpList::Get(const char *name)
	{
		if (KList::IsInt(name))
		{
			unsigned int index = (unsigned int) atoi(name);
			if (index >= 0)
				return this->At(index);
		}

		unsigned long hashval = zend_get_hash_value((char *) name, strlen(name));
		zval **copyval;

		if (zend_hash_quick_find(Z_ARRVAL_P(this->list),
					(char *) name,
					strlen(name),
					hashval,
					(void**)&copyval) == FAILURE)
		{
			return Value::Undefined;
		}

		SharedValue v = PhpUtils::ToKrollValue((zval *) copyval);
		return v;
	}

	void KPhpList::Set(const char *name, SharedValue value)
	{
		// Check for integer value as name
		int index = -1;
		if (KList::IsInt(name) && ((index = atoi(name)) >=0))
		{
			this->SetAt((unsigned int) index, value);
		}
		else
		{
			PhpUtils::AddKrollValueToPhpArray(value, this->list, name);
		}
	}

	bool KPhpList::Equals(SharedKObject other)
	{
		AutoPtr<KPhpList> phpOther = other.cast<KPhpList>();

		// This is not a PHP object
		if (phpOther.isNull())
			return false;

		return phpOther->ToPhp() == this->ToPhp();
	}

	SharedStringList KPhpList::GetPropertyNames()
	{
		SharedStringList property_names = new StringList();
		HashPosition pos;
		HashTable *ht = Z_ARRVAL_P(this->list);

		for (zend_hash_internal_pointer_reset_ex(ht, &pos);
				zend_hash_has_more_elements_ex(ht, &pos) == SUCCESS;
				zend_hash_move_forward_ex(ht, &pos))
		{
			char *key;
			unsigned int keylen;
			unsigned long index;

			zend_hash_get_current_key_ex(ht, &key, &keylen, &index, 0, &pos);

			property_names->push_back(new std::string(key));
		}

		zend_hash_destroy(ht);
		FREE_HASHTABLE(ht);

		return property_names;
	}

	unsigned int KPhpList::Size()
	{
		/*TODO: Implement*/
		return 0;
	}

	void KPhpList::Append(SharedValue value)
	{
		/*TODO: Implement*/
	}

	void KPhpList::SetAt(unsigned int index, SharedValue value)
	{
		/*TODO: Implement*/
	}

	bool KPhpList::Remove(unsigned int index)
	{
		/*TODO: Implement*/
		return true;
	}

	SharedValue KPhpList::At(unsigned int index)
	{
		/*TODO: Implement*/
		return Value::Null;
	}

	zval* KPhpList::ToPhp()
	{
		return this->list;
	}
}
