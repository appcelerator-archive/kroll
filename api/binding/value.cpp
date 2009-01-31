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
	SharedValue Value::NewInt(int value) { SharedValue v(new Value()); v->init(); v->SetInt(value); return v; }
	SharedValue Value::NewDouble(double value) { SharedValue v(new Value()); v->init(); v->SetDouble(value); return v; }
	SharedValue Value::NewBool(bool value) { SharedValue v(new Value()); v->init(); v->SetBool(value); return v; }
	SharedValue Value::NewString(const char* value) { SharedValue v(new Value()); v->init(); v->SetString(value); return v; }
	SharedValue Value::NewString(std::string& value) { SharedValue v(new Value()); v->init(); v->SetString(value.c_str()); return v; }
	SharedValue Value::NewList(SharedBoundList value) { SharedValue v(new Value()); v->init(); v->SetList(value); return v; }
	SharedValue Value::NewMethod(SharedBoundMethod method) { SharedValue v(new Value()); v->init(); v->SetMethod(method); return v; }
	SharedValue Value::NewObject(SharedBoundObject value) { SharedValue v(new Value()); v->init(); v->SetObject(value); return v; }
	SharedValue Value::NewObject(SharedPtr<StaticBoundObject> value) { SharedValue v(new Value()); v->init(); v->SetStaticBoundObject(value); return v; }
	Value::Value(SharedValue value) { init(); this->SetValue(value); }
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
			this->SetValue((Value*)&value);
		}
	}

	Value::~Value()
	{
		if (IsString())
		{
			delete this->stringValue;
			this->stringValue = NULL;
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
		this->objectValue = NULL;
		this->stringValue = NULL;
	}

	void Value::defaults()
	{
		if (this->IsString())
		{
			delete this->stringValue;
			this->stringValue = NULL;
		}
		this->type = UNDEFINED;
		this->objectValue = NULL;
		this->stringValue = NULL;
	}

	bool Value::IsInt() const { return type == INT || (type == DOUBLE && ((int) numberValue) == numberValue); }
	bool Value::IsDouble() const { return type == DOUBLE; }
	bool Value::IsNumber() const { return type == DOUBLE || type == INT; }
	bool Value::IsBool() const { return type == BOOL; }
	bool Value::IsString() const {  return type == STRING; }
	bool Value::IsList() const { return type == LIST; }
	bool Value::IsObject() const { return type == OBJECT; }
	bool Value::IsMethod() const { return type == METHOD; }
	bool Value::IsNull() const { return type == NULLV; }
	bool Value::IsUndefined() const { return type == UNDEFINED; }

	int Value::ToInt() const { return (int) numberValue; }
	double Value::ToDouble() const { return numberValue; }
	double Value::ToNumber() const { return numberValue; }
	bool Value::ToBool() const { return boolValue; }
	const char* Value::ToString() const { return stringValue; }
	SharedBoundObject Value::ToObject() const { return objectValue; }
	SharedBoundMethod Value::ToMethod() const { return objectValue.cast<BoundMethod>(); }
	SharedBoundList Value::ToList() const { return objectValue.cast<BoundList>(); }

	void Value::SetValue(SharedValue other)
	{
		SetValue(other.get());
	}

	void Value::SetValue(Value *other)
	{
		if (other->IsInt())
			this->SetInt(other->ToInt());

		else if (other->IsDouble())
			this->SetDouble(other->ToDouble());

		else if (other->IsBool())
			this->SetBool(other->ToBool());

		else if (other->IsString())
			this->SetString(other->ToString());

		else if (other->IsList())
			this->SetList(other->ToList());

		else if (other->IsMethod())
			this->SetMethod(other->ToMethod());

		else if (other->IsObject())
			this->SetObject(other->ToObject());

		else if (other->IsNull())
			this->SetNull();

		else if (other->IsUndefined())
			this->SetUndefined();

		else
			throw "Error on set. Unknown type for other";
	}

	void Value::SetInt(int value)
	{
		defaults();
		this->numberValue = value;
		type = INT;
	}

	void Value::SetDouble(double value)
	{
		defaults();
		this->numberValue = value;
		type = DOUBLE;
	}

	void Value::SetBool(bool value)
	{
		defaults();
		this->boolValue = value;
		type = BOOL;
	}

	void Value::SetString(const char* value)
	{
		defaults();
		this->stringValue = strdup(value);
		type = STRING;
	}

	void Value::SetList(SharedBoundList value)
	{
		defaults();
		this->objectValue = value;
		type = LIST;
	}

	void Value::SetObject(SharedBoundObject value)
	{
		defaults();
		this->objectValue = value;
		type = OBJECT;
	}

	void Value::SetStaticBoundObject(SharedPtr<StaticBoundObject> value)
	{
		defaults();
		this->objectValue = value;
		type = OBJECT;
	}

	void Value::SetMethod(SharedBoundMethod value)
	{
		defaults();
		this->objectValue = value;
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
