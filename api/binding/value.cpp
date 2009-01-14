/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <sstream>

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
		init();
		this->type = value.type;
		if (value.IsString())
		{
			// make a copy
			this->value.stringValue = new std::string(value.ToString());
		}
		else
		{
			this->value = value.value;
			if (value.IsObject())
			{
				KR_ADDREF(this->value.objectValue);
			}
		}
	}

	Value::~Value()
	{
		if (IsObject())
		{
			KR_DECREF(this->value.objectValue);
		}
		else if (IsString())
		{
			delete this->value.stringValue;
			this->value.stringValue = 0;
		}
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
		this->type = UNDEFINED;
		this->value.objectValue = 0;
		this->value.stringValue = 0;
	}

	void Value::defaults()
	{
		if (this->IsObject())
		{
			KR_DECREF(this->value.objectValue);
		}
		else if (this->IsString())
		{
			delete this->value.stringValue;
			this->value.stringValue = 0;
		}
		this->type = UNDEFINED;
		this->value.objectValue = 0;
		this->value.stringValue = 0;
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

	int Value::ToInt() const { return (int) value.numberValue; }
	double Value::ToDouble() const { return value.numberValue; }
	bool Value::ToBool() const { return value.boolValue; }
	std::string Value::ToString() const { return std::string(*value.stringValue); }
	BoundObject* Value::ToObject() const { return value.objectValue; }
	BoundMethod* Value::ToMethod() const { return (BoundMethod*) value.objectValue; }
	BoundList* Value::ToList() const { return (BoundList*) value.objectValue; }

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
		this->value.numberValue = value;
		type = INT;
	}

	void Value::Set(double value)
	{
		defaults();
		this->value.numberValue = value;
		type = DOUBLE;
	}

	void Value::Set(bool value)
	{
		defaults();
		this->value.boolValue = value;
		type = BOOL;
	}

	void Value::Set(std::string value)
	{
		defaults();
		this->value.stringValue = new std::string(value);
		type = STRING;
	}

	void Value::Set(BoundList* value)
	{
		defaults();
		this->value.objectValue = value;
		KR_ADDREF(this->value.objectValue);
		type = LIST;
	}

	void Value::Set(BoundObject* value)
	{
		defaults();
		this->value.objectValue = value;
		KR_ADDREF(this->value.objectValue);
		type = OBJECT;
	}

	void Value::Set(BoundMethod* value)
	{
		defaults();
		this->value.objectValue = value;
		KR_ADDREF(this->value.objectValue);
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

	/* TODO: when the shared_ptr stuff comes through
	 * BoundObject,List,Method should have their own
	 * DisplayString impls */
	char* Value::DisplayString()
	{
		std::ostringstream oss;

		if (this->IsInt() )
		{
			oss << this->ToInt() << "i";
		}
		else if (this->IsDouble())
		{
			oss << this->ToDouble() << "d";
		}
		else if (this->IsBool())
		{
			if (this->ToBool())
				oss << "true";
			else
				oss << "false";
		}
		else if (this->IsString())
		{
			oss << "\"" << this->ToString() << "\"";
		}
		else if (this->IsList())
		{
			BoundList *list = this->ToList();
			oss << "[";
			if (list->Size() > 0)
			{
				oss << list->At(0)->DisplayString();
				for (int i = 1; i < list->Size(); i++)
				{
					oss << ", " << list->At(i)->DisplayString();
				}
			}
			oss << "]";
		}
		else if (this->IsObject())
		{
			BoundObject *obj = this->ToObject();
			std::vector<const char *> props;
			obj->GetPropertyNames(&props);

			oss << "{";
			if (props.size() > 0)
			{
				oss << props.at(0) << " : "
				    << obj->Get(props.at(0))->DisplayString();
				for (size_t i = 1; i < props.size(); i++)
				{
					oss << ", " << props.at(i) <<
					       " : " <<
					       obj->Get(props.at(i))->DisplayString();
				}
			}
			oss << "}";
		}
		else if (this->IsMethod())
		{
			oss << "<method>";
		}
		else if (this->IsNull())
		{
			oss << "<null>";
		}
		else if (this->IsUndefined())
		{
			oss << "<undefined>";
		}
		else
		{
			oss << "<unknown>";
		}

		return strdup(oss.str());

	}

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

}
