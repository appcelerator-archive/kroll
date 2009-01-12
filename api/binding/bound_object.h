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
#include "../file_utils.h"

extern KROLL_API kroll::RefCounted* CreateEmptyBoundObject();

namespace kroll
{
	/*
		Class: BoundObject
	*/
	class KROLL_API BoundObject : public RefCounted
	{
	public:
		/*
			Constructor: BoundObject
		*/
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
		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference,
		  if they increase the reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value) = 0;

		/*
			Function: Get

		  Return an object's property. The returned value is automatically
		  reference counted and must be released if the callee does not hold
		  a reference (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name) = 0;

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual void GetPropertyNames(std::vector<const char *> *property_names) = 0;

		/*
			Function: SetNS

			TODO: Document me
		*/
		void SetNS(const char *name, Value* value)
		{
			std::vector<std::string> tokens;
			std::string s(name);
			FileUtils::Tokenize(s,tokens,".");
			if (tokens.size()==1)
			{
				this->Set(name,value);
				return;
			}
			BoundObject *scope = this;
			for (int c=0;c<(int)tokens.size()-1;c++)
			{
				std::string token = tokens.at(c);
				Value *newscope = scope->Get(token.c_str());
				if (newscope->IsUndefined() || newscope->IsNull())
				{
					BoundObject* bo = (BoundObject*)CreateEmptyBoundObject();
					Value *newvalue = new Value(bo);
					scope->Set(token.c_str(),newvalue);
					KR_DECREF(newscope); // OK to release since scope holds
					KR_DECREF(newvalue);
					scope = bo;
				}
				else if (newscope->IsObject())
				{
					scope = newscope->ToObject();
				}
				else
				{
					Value *error = new Value("Invalid namespace on setNS");
					throw error;
				}
			}
			std::string token = tokens.at(tokens.size()-1);
#ifdef DEBUG_BINDING
			std::cout << "BIND: " << value->ToTypeString() << " to: " << name << std::endl;
#endif
			scope->Set(token.c_str(),value);
		}

		/*
			Function: GetNS
			
			TODO: Document me
		*/
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
				if (current->IsObject())
				{
					scope = current->ToObject();
				}
				else
				{
					return Value::Undefined();
				}
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
