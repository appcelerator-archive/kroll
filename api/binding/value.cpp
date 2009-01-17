/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <sstream>
#include <cstring>

namespace kroll
{
	Value::Value() { init(); SetUndefined(); }
	Value::Value(int value) { init(); this->Set(value); }
	Value::Value(double value) { init(); this->Set(value); }
	Value::Value(bool value) { init(); this->Set(value); }
	Value::Value(const char* value) { init(); this->Set(value); }
	Value::Value(std::string& value) { init(); this->Set(value.c_str()); }
	Value::Value(SharedBoundList value) { init(); this->Set(value); }
	Value::Value(SharedBoundMethod method) { init(); this->Set(method); }
	Value::Value(SharedBoundObject value) { init(); this->Set(value); }
	Value::Value(SharedPtr<StaticBoundObject> value) { init(); this->Set(value); }
	Value::Value(SharedValue value) { init(); this->Set(value); }
	Value::Value(const Value& value)
	{
		init();
		this->type = value.type;
		if (value.IsString())
		{
			// make a copy
			this->stringValue = strdup(value.ToString());
		}
		else
		{
			this->Set((Value*)&value);
			//if (value.IsObject())
			//{
			//	KR_ADDREF(this->value.objectValue);
			//}
		}
	}

	Value::~Value()
	{
		if (IsObject())
		{
			//KR_DECREF(this->value.objectValue);
		}
		else if (IsString())
		{
			delete this->stringValue;
			this->stringValue = 0;
		}
	}

	SharedValue Value::Undefined = CreateUndefined();
	SharedValue Value::Null = CreateNull();

	SharedValue Value::CreateUndefined()
	{
		SharedValue v = new Value();
		v->SetUndefined();
		return v;
	}

	SharedValue Value::CreateNull()
	{
		SharedValue v = new Value();
		v->SetNull();
		return v;
	}

	void Value::init()
	{
		this->type = UNDEFINED;
		this->objectValue = 0;
		this->stringValue = 0;
	}

	void Value::defaults()
	{
		if (this->IsObject())
		{
			//KR_DECREF(this->value.objectValue);
		}
		else if (this->IsString())
		{
			delete this->stringValue;
			this->stringValue = 0;
		}
		this->type = UNDEFINED;
		this->objectValue = 0;
		this->stringValue = 0;
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
	const char* Value::ToString() const { return stringValue; }
	SharedBoundObject Value::ToObject() const { return objectValue; }
	SharedBoundMethod Value::ToMethod() const { return objectValue.cast<BoundMethod>(); }
	SharedBoundList Value::ToList() const { return objectValue.cast<BoundList>(); }

	void Value::Set(SharedValue other)
	{
		Set(other.get());
	}

	void Value::Set(Value *other)
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
		this->numberValue = value;
		type = INT;
	}

	void Value::Set(double value)
	{
		defaults();
		this->numberValue = value;
		type = DOUBLE;
	}

	void Value::Set(bool value)
	{
		defaults();
		this->boolValue = value;
		type = BOOL;
	}

	void Value::Set(char* value)
	{
		defaults();
		this->stringValue = strdup(value);
		type = STRING;
	}

	void Value::Set(SharedBoundList value)
	{
		defaults();
		this->objectValue = value;
		//KR_ADDREF(this->value.objectValue);
		type = LIST;
	}

	void Value::Set(SharedBoundObject value)
	{
		defaults();
		this->objectValue = value;
		//KR_ADDREF(this->value.objectValue);
		type = OBJECT;
	}

	void Value::Set(SharedPtr<StaticBoundObject> value)
	{
		defaults();
		this->objectValue = value;
		type = OBJECT;
	}

	void Value::Set(SharedBoundMethod value)
	{
		defaults();
		this->objectValue = value;
		//KR_ADDREF(this->value.objectValue);
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
			SharedBoundList tlist = this->ToList();
			SharedBoundList olist = i.ToList();

			if (tlist->Size() != olist->Size())
				return false;

			for (int i = 0; i < (int) tlist->Size(); i++)
			{
				SharedValue a = tlist->At(i);
				SharedValue b = olist->At(i);

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
	SharedString Value::DisplayString(int levels)
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
			SharedBoundList list = this->ToList();
			SharedString disp_string = list->DisplayString(levels-1);
			oss << *disp_string;
		}
		else if (this->IsObject())
		{
			SharedBoundObject obj = this->ToObject();
			SharedString disp_string = obj->DisplayString(levels-1);
			oss << *disp_string;
		}
		else if (this->IsMethod())
		{
			SharedBoundMethod method = this->ToMethod();
			oss << "<BoundMethod at " << method.get() << ">";
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

		return new std::string(oss.str());
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
