/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>

namespace kroll
{
	void KObject::Set(SharedString name, SharedValue value)
	{
		this->Set(name->c_str(), value);
	}

	SharedValue KObject::Get(SharedString name)
	{
		return this->Get(name->c_str());
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
				std::cerr << "invalid namespace for " << name << ", token: " << token << " was " << next_val->ToTypeString() << std::endl;
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
		std::cout << "BOUND: " << value->ToTypeString() << " to: " << name << std::endl;
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

	std::string KObject::GetString(const char * name, std::string defaultValue)
	{
		SharedValue prop = this->Get(name);
		if(!prop->IsUndefined() && prop->IsString())
		{
			return prop->ToString();
		}
		else
		{
			return defaultValue;
		}
	}

	bool KObject::GetBool(const char * name, bool defaultValue)
	{
		SharedValue prop = this->Get(name);
		if(!prop->IsUndefined())
		{
			return prop->ToBool();
		}
		else
		{
			return defaultValue;
		}
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

}

