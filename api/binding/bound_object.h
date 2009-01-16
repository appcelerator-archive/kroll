/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_OBJECT_H_
#define _KR_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>
#include "../scoped_dereferencer.h"
#include <Poco/SharedPtr.h>

using namespace Poco;

namespace kroll
{
	typedef std::vector<const char *> StringList;
	typedef SharedPtr<StringList> SharedStringList;

	/*
		Class: BoundObject
	*/
	class KROLL_API BoundObject : public RefCounted
	{
	public:
		/*
			Constructor: BoundObject
		*/
		BoundObject() { }
		virtual ~BoundObject() { }

		static SharedPtr<BoundObject> CreateEmptyBoundObject();
	public:
		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference,
		  if they increase the reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, SharedPtr<Value> value) = 0;

		/*
			Function: Get

		  Return an object's property. The returned value is automatically
		  reference counted and must be released if the callee does not hold
		  a reference (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedPtr<Value> Get(const char *name) = 0;

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/*
			Function: SetNS

			TODO: Document me
		*/
		void SetNS(const char *name, SharedPtr<Value> value)
		{
			std::vector<std::string> tokens;
			FileUtils::Tokenize(std::string(name), tokens, ".");

			SharedPtr<BoundObject> next;
			BoundObject* scope = this;
			for (size_t i = 0; i < tokens.size() - 1; i++)
			{
				// Ensure dereference, except for "this" object
				ScopedDereferencer s_dec(scope);
				if (scope == this) KR_ADDREF(scope);
				const char* token = tokens[i].c_str();
				SharedPtr<BoundObject> next;
				SharedPtr<Value> next_val = scope->Get(token);
				if (next_val->IsUndefined())
				{
					next = BoundObject::CreateEmptyBoundObject();
					next_val = new Value(next);
					//ScopedDereferencer next_val_dec2(next_val);
					scope->Set(token, next_val);

				}
				else if (!next_val->IsObject()
				         && !next_val->IsMethod()
				         && !next_val->IsList())
				{
					throw new Value("Invalid namespace on setNS");

				}
				else
				{
					next = next_val->ToObject();
				}

				scope = next.get();
			}

			const char *prop_name = tokens[tokens.size()-1].c_str();
			scope->Set(prop_name, value);

#ifdef DEBUG_BINDING
			std::cout << "BOUND: " << value->ToTypeString() << " to: " << name << std::endl;
#endif
		}

		/*
			Function: GetNS

			TODO: Document me
		*/
		SharedPtr<Value> GetNS(const char *name)
		{
			std::string s(name);
			std::string::size_type last = 0;
			std::string::size_type pos = s.find_first_of(".");
			SharedPtr<Value> current;
			BoundObject* scope = this;
			while (pos != std::string::npos)
			{
				std::string token = s.substr(last,pos);
				current = scope->Get(token.c_str());
				last = pos + 1;
			    pos = s.find_first_of(".", last);
				if (current->IsObject())
				{
					scope = current->ToObject().get();
				}
				else
				{
					return Value::Undefined;
				}
			}
			if (pos!=s.length())
			{
				std::string token = s.substr(last);
				current = scope->Get(token.c_str());
			}

			return current;
		}

	private:
		DISALLOW_EVIL_CONSTRUCTORS(BoundObject);
	};

}

#endif
