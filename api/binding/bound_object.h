/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_OBJECT_H_
#define _KR_BOUND_OBJECT_H_

#include "binding.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	class KROLL_API BoundObject : public RefCounted
	{
	public:
		BoundObject(BoundObject *scope_) : scope(scope_) 
		{ 
			KR_ADDREF(scope); 
		}
		BoundObject() : scope(NULL) {}
	protected:
		virtual ~BoundObject() 
		{
			KR_DECREF(scope);
		}
	public:
		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference,
		 * if they increase the reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value) = 0;

		/**
		 * Return an object's property. The returned value is automatically
		 * reference counted and must be released if the callee does not hold
		 * a reference (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name) = 0;

		/**
		 * Return a list of this object's property names.
		 */
		virtual std::vector<std::string> GetPropertyNames() = 0;

		void SetNS(const char *name, Value* value)
		{
			std::string s(name);
			std::string::size_type pos = s.find_first_of(".");
			if (pos==std::string::npos)
			{
				this->Set(name,value);
				return;
			}
			std::string::size_type last = 0;
			Value* current = NULL;
			BoundObject* scope = this;
			while (pos != std::string::npos) 
			{
				std::string token = s.substr(last,pos);
				std::cout << "getting token = " << token << std::endl;
				current = scope->Get(token.c_str());
				last = pos + 1;
			    pos = s.find_first_of(".", last);
				if (!current->IsObject())
				{
					break;
				}
				scope = current->ToObject();
			}
			if (pos!=s.length())
			{
				std::string token = s.substr(last);
				std::cout << "setting token = " << token << std::endl;
				scope->Set(token.c_str(),value);
			}
		}
		Value* GetNS(const char *name)
		{
			std::string s(name);
			std::string::size_type last = 0;
			std::string::size_type pos = s.find_first_of(".");
			Value* current = NULL;
			BoundObject* scope = this;
			while (pos != std::string::npos) 
			{
				std::string token = s.substr(last,pos);
				current = scope->Get(token.c_str());
				last = pos + 1;
			    pos = s.find_first_of(".", last);
				if (!current->IsObject())
				{
					break;
				}
				scope = current->ToObject();
			}
			if (pos!=s.length())
			{
				std::string token = s.substr(last);
				current = scope->Get(token.c_str());
			}
			return current;
		}

	protected:
		BoundObject *scope;
		
	private:
		DISALLOW_EVIL_CONSTRUCTORS(BoundObject);
	};

}

#endif
