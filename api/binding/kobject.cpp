/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>

namespace kroll
{

	bool KObject::Equals(KObjectRef other)
	{
		return other.get() == this;
	}

	bool KObject::HasProperty(const char* name)
	{
		SharedStringList names = this->GetPropertyNames();
		for (size_t i = 0; i < names->size(); i++)
		{
			if (!strcmp(name, names->at(i)->c_str()))
				return true;
		}
		return false;
	}

	SharedString KObject::DisplayString(int levels)
	{
		std::stringstream ss;

		if (levels == 0)
		{
			ss << "<KObject at " << this << ">";
		}
		else
		{
			SharedStringList props = this->GetPropertyNames();
			ss << "{";
			for (size_t i = 0; i < props->size(); i++)
			{
				KValueRef prop = this->Get(props->at(i));
				SharedString disp_string = prop->DisplayString(levels);

				ss << " " << *(props->at(i))
				    << " : " << *disp_string << ",";
			}

			if (props->size() > 0) // Erase last comma
				ss.seekp((int)ss.tellp() - 1);

			ss << "}";
		}

		return new std::string(ss.str());
	}

	void KObject::Set(SharedString name, KValueRef value)
	{
		this->Set(name->c_str(), value);
	}

	KValueRef KObject::Get(SharedString name)
	{
		return this->Get(name->c_str());
	}

	int KObject::GetInt(const char* name, int defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsInt())
		{
			return prop->ToInt();
		}
		else
		{
			return defaultValue;
		}
	}

	double KObject::GetDouble(const char* name, double defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsDouble())
		{
			return prop->ToDouble();
		}
		else
		{
			return defaultValue;
		}
	}

	double KObject::GetNumber(const char* name, double defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsNumber())
		{
			return prop->ToNumber();
		}
		else
		{
			return defaultValue;
		}
	}

	bool KObject::GetBool(const char* name, bool defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsBool())
		{
			return prop->ToBool();
		}
		else
		{
			return defaultValue;
		}
	}

	std::string KObject::GetString(const char* name, std::string defaultValue)
	{
		KValueRef prop = this->Get(name);
		if(prop->IsString())
		{
			return prop->ToString();
		}
		else
		{
			return defaultValue;
		}
	}

	KObjectRef KObject::GetObject(const char* name, KObjectRef defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsObject())
		{
			return prop->ToObject();
		}
		else
		{
			return defaultValue;
		}
	}

	KMethodRef KObject::GetMethod(const char* name, KMethodRef defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsMethod())
		{
			return prop->ToMethod();
		}
		else
		{
			return defaultValue;
		}
	}

	KListRef KObject::GetList(const char* name, KListRef defaultValue)
	{
		KValueRef prop = this->Get(name);
		if (prop->IsList())
		{
			return prop->ToList();
		}
		else
		{
			return defaultValue;
		}
	}

	void KObject::SetUndefined(const char *name)
	{
		this->Set(name, Value::Undefined);
	}

	void KObject::SetNull(const char *name)
	{
		this->Set(name, Value::Null);
	}

	void KObject::SetInt(const char *name, int v)
	{
		KValueRef val = Value::NewInt(v);
		this->Set(name, val);
	}

	void KObject::SetDouble(const char *name, double v)
	{
		KValueRef val = Value::NewDouble(v);
		this->Set(name, val);
	}

	void KObject::SetNumber(const char *name, double v)
	{
		KValueRef val = Value::NewDouble(v);
		this->Set(name, val);
	}

	void KObject::SetBool(const char *name, bool v)
	{
		KValueRef val = Value::NewBool(v);
		this->Set(name, val);
	}

	void KObject::SetString(const char *name, std::string v)
	{
		KValueRef val = Value::NewString(v);
		this->Set(name, val);
	}

	void KObject::SetObject(const char *name, KObjectRef object)
	{
		KValueRef obj_val = Value::NewObject(object);
		this->Set(name, obj_val);
	}

	void KObject::SetMethod(const char *name, KMethodRef object)
	{
		KValueRef obj_val = Value::NewMethod(object);
		this->Set(name, obj_val);
	}

	void KObject::SetList(const char *name, KListRef object)
	{
		KValueRef obj_val = Value::NewList(object);
		this->Set(name, obj_val);
	}

	void KObject::GetStringList(const char *name, std::vector<std::string> &list)
	{
		KValueRef prop = this->Get(name);
		if(!prop->IsUndefined() && prop->IsList())
		{
			KListRef values = prop->ToList();
			if (values->Size() > 0)
			{
				for (unsigned int c = 0; c < values->Size(); c++)
				{
					KValueRef v = values->At(c);
					if (v->IsString())
					{
						const char *s = v->ToString();
						list.push_back(s);
					}
				}
			}
		}
	}

	void KObject::SetNS(const char *name, KValueRef value)
	{
		std::vector<std::string> tokens;
		FileUtils::Tokenize(std::string(name), tokens, ".");

		KObject *scope = this;
		for (size_t i = 0; i < tokens.size() - 1; i++)
		{
			const char* token = tokens[i].c_str();
			StaticBoundObject *next;
			KValueRef next_val = scope->Get(token);

			if (next_val->IsUndefined())
			{
				next = new StaticBoundObject();
				KObjectRef so = next;
				next_val = Value::NewObject(so);
				scope->Set(token, next_val);
				scope = next;
			}
			else if (!next_val->IsObject()
			         && !next_val->IsMethod()
			         && !next_val->IsList())
			{
				std::cerr << "invalid namespace for " << name << ", token: " << token << " was " << next_val->GetType() << std::endl;
				throw Value::NewString("Invalid namespace on setNS");
			}
			else
			{
				scope = next_val->ToObject().get();
			}
		}

		const char *prop_name = tokens[tokens.size()-1].c_str();
		scope->Set(prop_name, value);

#ifdef DEBUG_BINDING
		std::cout << "BOUND: " << value->GetType() << " to: " << name << std::endl;
#endif
	}

	KValueRef KObject::GetNS(const char *name)
	{
		std::string s(name);
		std::string::size_type last = 0;
		std::string::size_type pos = s.find_first_of(".");
		KValueRef current;
		KObject* scope = this;
		while (pos != std::string::npos)
		{
			std::string token = s.substr(last,pos-last);
			current = scope->Get(token.c_str());
			if (current->IsObject())
			{
				scope = current->ToObject().get();
			}
			else
			{
				return Value::Undefined;
			}
			last = pos + 1;
		    pos = s.find_first_of(".", last);
		}
		if (pos!=s.length())
		{
			std::string token = s.substr(last);
			current = scope->Get(token.c_str());
		}

		return current;
	}

	KValueRef KObject::CallNS(const char *name)
	{
		ValueList args;
		return CallNS(name, args);
	}
	
	KValueRef KObject::CallNS(const char *name, KValueRef val1)
	{
		ValueList args;
		args.push_back(val1);
		return CallNS(name, args);
	}

	KValueRef KObject::CallNS(const char *name, KValueRef val1, KValueRef val2)
	{
		ValueList args;
		args.push_back(val1);
		args.push_back(val2);
		return CallNS(name, args);
	}

	KValueRef KObject::CallNS(const char *name, KValueRef val1, KValueRef val2, KValueRef val3)
	{
		ValueList args;
		args.push_back(val1);
		args.push_back(val2);
		args.push_back(val3);
		return CallNS(name, args);
	}

	KValueRef KObject::CallNS(const char *name, const ValueList& args)
	{
		KValueRef callable_value = GetNS(name);
		if (callable_value->IsUndefined()) {
			return callable_value;
		}

		if (!callable_value->IsMethod()) {
			return Value::Undefined;
		}

		return callable_value->ToMethod()->Call(args);
	}

	std::string& KObject::GetType()
	{
		return type;
	}

	KObjectRef KObject::Unwrap(KObjectRef o)
	{
		AutoPtr<ProfiledBoundObject> pobj = o.cast<ProfiledBoundObject>();
		if (pobj.isNull())
		{
			return o;
		}
		else
		{
			return pobj->GetDelegate();
		}
	}

}

