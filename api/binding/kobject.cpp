/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>

namespace kroll
{

	bool KObject::Equals(SharedKObject other)
	{
		return other.get() == this;
	}

	bool KObject::HasProperty(const char* name)
	{
		SharedStringList names = this->GetPropertyNames();
		for (size_t i = 0; i < names->size(); i++) {
			if (!strcmp(name, names->at(i)->c_str())) {
				return true;
			}
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
				SharedValue prop = this->Get(props->at(i));
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

	void KObject::Set(SharedString name, SharedValue value)
	{
		this->Set(name->c_str(), value);
	}

	SharedValue KObject::Get(SharedString name)
	{
		return this->Get(name->c_str());
	}

	int KObject::GetInt(const char* name, int defaultValue)
	{
		SharedValue prop = this->Get(name);
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
		SharedValue prop = this->Get(name);
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
		SharedValue prop = this->Get(name);
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
		SharedValue prop = this->Get(name);
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
		SharedValue prop = this->Get(name);
		if(prop->IsString())
		{
			return prop->ToString();
		}
		else
		{
			return defaultValue;
		}
	}

	SharedKObject KObject::GetObject(const char* name, SharedKObject defaultValue)
	{
		SharedValue prop = this->Get(name);
		if (prop->IsObject())
		{
			return prop->ToObject();
		}
		else
		{
			return defaultValue;
		}
	}

	SharedKMethod KObject::GetMethod(const char* name, SharedKMethod defaultValue)
	{
		SharedValue prop = this->Get(name);
		if (prop->IsMethod())
		{
			return prop->ToMethod();
		}
		else
		{
			return defaultValue;
		}
	}

	SharedKList KObject::GetList(const char* name, SharedKList defaultValue)
	{
		SharedValue prop = this->Get(name);
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
		SharedValue val = Value::NewInt(v);
		this->Set(name, val);
	}

	void KObject::SetDouble(const char *name, double v)
	{
		SharedValue val = Value::NewDouble(v);
		this->Set(name, val);
	}

	void KObject::SetNumber(const char *name, double v)
	{
		SharedValue val = Value::NewDouble(v);
		this->Set(name, val);
	}

	void KObject::SetBool(const char *name, bool v)
	{
		SharedValue val = Value::NewBool(v);
		this->Set(name, val);
	}

	void KObject::SetString(const char *name, std::string v)
	{
		SharedValue val = Value::NewString(v);
		this->Set(name, val);
	}

	void KObject::SetObject(const char *name, SharedKObject object)
	{
		SharedValue obj_val = Value::NewObject(object);
		this->Set(name, obj_val);
	}

	void KObject::SetMethod(const char *name, SharedKMethod object)
	{
		SharedValue obj_val = Value::NewMethod(object);
		this->Set(name, obj_val);
	}

	void KObject::SetList(const char *name, SharedKList object)
	{
		SharedValue obj_val = Value::NewList(object);
		this->Set(name, obj_val);
	}

	void KObject::GetStringList(const char *name, std::vector<std::string> &list)
	{
		SharedValue prop = this->Get(name);
		if(!prop->IsUndefined() && prop->IsList())
		{
			SharedKList values = prop->ToList();
			if (values->Size() > 0)
			{
				for (unsigned int c = 0; c < values->Size(); c++)
				{
					SharedValue v = values->At(c);
					if (v->IsString())
					{
						const char *s = v->ToString();
						list.push_back(s);
					}
				}
			}
		}
	}

	void KObject::SetNS(const char *name, SharedValue value)
	{
		std::vector<std::string> tokens;
		FileUtils::Tokenize(std::string(name), tokens, ".");

		KObject *scope = this;
		for (size_t i = 0; i < tokens.size() - 1; i++)
		{
			const char* token = tokens[i].c_str();
			StaticBoundObject *next;
			SharedValue next_val = scope->Get(token);

			if (next_val->IsUndefined())
			{
				next = new StaticBoundObject();
				SharedKObject so = next;
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

	SharedValue KObject::GetNS(const char *name)
	{
		std::string s(name);
		std::string::size_type last = 0;
		std::string::size_type pos = s.find_first_of(".");
		SharedValue current;
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

	SharedValue KObject::CallNS(const char *name, SharedValue val1)
	{
		ValueList args;
		args.push_back(val1);
		return CallNS(name, args);
	}

	SharedValue KObject::CallNS(const char *name, SharedValue val1, SharedValue val2)
	{
		ValueList args;
		args.push_back(val1);
		args.push_back(val2);
		return CallNS(name, args);
	}

	SharedValue KObject::CallNS(const char *name, SharedValue val1, SharedValue val2, SharedValue val3)
	{
		ValueList args;
		args.push_back(val1);
		args.push_back(val2);
		args.push_back(val3);
		return CallNS(name, args);
	}

	SharedValue KObject::CallNS(const char *name, const ValueList& args)
	{
		SharedValue callable_value = GetNS(name);
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

	SharedKObject KObject::Unwrap(SharedKObject o)
	{
		SharedPtr<ProfiledBoundObject> pobj = o.cast<ProfiledBoundObject>();
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

