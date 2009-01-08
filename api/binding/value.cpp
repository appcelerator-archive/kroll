/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"

namespace kroll
{
	Value::Value() { init(); SetUndefined(); }
	Value::Value(int value) { init(); this->Set(value); }
	Value::Value(double value) { init(); this->Set(value); }
	Value::Value(bool value) { init(); this->Set(value); }
	Value::Value(const char* value) { init(); this->Set(std::string(value)); }
	Value::Value(std::string& value) { init(); this->Set(value); }
	Value::Value(BoundList* value) { init(); this->Set(value); }
	Value::Value(BoundMethod* method) { init(); this->Set(method); }
	Value::Value(BoundObject* value) { init(); this->Set(value); }
	Value::Value(const Value& value)
	{
		this->type = value.type;
		this->numberValue = value.numberValue;
		this->boolValue = value.boolValue;
		this->stringValue = value.stringValue;
		this->objectValue = value.objectValue;
		KR_ADDREF(this->objectValue);
	}

	Value::~Value()
	{
		KR_DECREF(this->objectValue);
	}

	Value* Value::Undefined()
	{
		Value* v = new Value();
		v->SetUndefined();
		return v;
	}

	Value* Value::Null()
	{
		Value* v = new Value();
		v->SetNull();
		return v;
	}

	void Value::init()
	{
		this->objectValue = NULL;
	}

	void Value::defaults()
	{
		this->type = UNDEFINED;
		this->numberValue = 0;
		this->boolValue = false;
		this->stringValue = "";
		KR_DECREF(this->objectValue);
	}

	bool Value::IsInt() const { return type == INT; }
	bool Value::IsDouble() const { return type == DOUBLE; }
	bool Value::IsBool() const { return type == BOOL; }
	bool Value::IsString() const { return type == STRING; }
	bool Value::IsList() const { return type == LIST; }
	bool Value::IsObject() const { return type == OBJECT; }
	bool Value::IsMethod() const { return type == METHOD; }
	bool Value::IsNull() const { return type == NULLV; }
	bool Value::IsUndefined() const { return type == UNDEFINED; }

	int Value::ToInt() const { return (int) numberValue; }
	double Value::ToDouble() const { return numberValue; }
	bool Value::ToBool() const { return boolValue; }
	std::string Value::ToString() const { return stringValue; }
	BoundObject* Value::ToObject() const { return objectValue; }
	BoundMethod* Value::ToMethod() const { return (BoundMethod*) objectValue; }
	BoundList* Value::ToList() const { return (BoundList*) objectValue; }

	const char* Value::ToTypeString()
	{
		if (IsInt())
			return "INT";

		else if (IsDouble())
			return "DOUBLE";

		else if (IsBool())
			return "BOOL";

		else if (IsString())
			return "STRING";

		else if (IsList())
			return "LIST";

		else if (IsObject())
			return "OBJECT";

		else if (IsMethod())
			return "METHOD";

		else if (IsNull())
			return "NULL";

		else if (IsUndefined())
			return "UNDEFINED";

		fprintf(stderr, "ERROR: unknown type:%d\n",type);
		return "UNKNOWN";
	}

	void Value::Set(Value* other)
	{
		if (other->IsInt())
			this->Set(other->ToInt());

		else if (other->IsDouble())
			this->Set(other->ToDouble());

		else if (other->IsBool())
			this->Set(other->ToBool());

		else if (other->IsString())
			this->Set(other->ToString());

		else if (other->IsList())
			this->Set(other->ToList());

		else if (other->IsMethod())
			this->Set(other->ToMethod());

		else if (other->IsObject())
			this->Set(other->ToObject());

		else if (other->IsNull())
			this->SetNull();

		else if (other->IsUndefined())
			this->SetUndefined();

		else
			throw "Error on set. Unknown type for other";
	}

	void Value::Set(int value)
	{
		defaults();
		numberValue = value;
		type = INT;
	}

	void Value::Set(double value)
	{
		defaults();
		numberValue = value;
		type = DOUBLE;
	}

	void Value::Set(bool value)
	{
		defaults();
		boolValue = value;
		type = BOOL;
	}

	void Value::Set(std::string value)
	{
		defaults();
		stringValue = value;
		type = STRING;
	}

	void Value::Set(BoundList* value)
	{
		defaults();
		objectValue = value;
		KR_ADDREF(objectValue);
		type = LIST;
	}

	void Value::Set(BoundObject* value)
	{
		defaults();
		objectValue = value;
		KR_ADDREF(objectValue);
		type = OBJECT;
	}

	void Value::Set(BoundMethod* value)
	{
		defaults();
		objectValue = value;
		KR_ADDREF(objectValue);
		type = METHOD;
	}

	void Value::SetNull()
	{
		defaults();
		type = NULLV;
	}

	void Value::SetUndefined()
	{
		defaults();
		type = UNDEFINED;
	}

	bool Value::operator== (Value i)
	{
		if (this->IsInt() && i.IsInt() && this->ToInt() == i.ToInt())
			return true;
		if (this->IsDouble() && i.IsDouble() && this->ToDouble() == i.ToDouble())
			return true;
		if (this->IsBool() && i.IsBool() && this->ToBool() == i.ToBool())
			return true;
		if (this->IsString() && i.IsString() && this->ToString() == i.ToString())
			return true;
		if (this->IsObject() && i.IsObject() && this->ToObject() == i.ToObject())
			return true;
		if (this->IsMethod() && i.IsMethod() && this->ToMethod() == i.ToMethod())
			return true;
		if (this->IsNull() && i.IsNull())
			return true;
		if (this->IsUndefined() && i.IsUndefined())
			return true;

		if (this->IsList() && i.IsList())
		{
			BoundList* tlist = this->ToList();
			BoundList* olist = i.ToList();

			if (tlist->Size() != olist->Size())
				return false;

			for (int i = 0; i < (int) tlist->Size(); i++)
			{
				Value *a = tlist->At(i);
				ScopedDereferencer sa(a);
				Value *b = olist->At(i);
				ScopedDereferencer sb(b);

				if (a != b)
					return false;
			}

			return true;
		}

		return false;
	}
}
