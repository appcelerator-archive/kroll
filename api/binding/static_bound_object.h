/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_OBJECT_H_
#define _KR_STATIC_BOUND_OBJECT_H_

#include "binding.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	/*	
		Class: StaticBoundObject
	*/
	class KROLL_API StaticBoundObject : public BoundObject
	{
	public:
		/*
			Constructor: StaticBoundObject
		*/
		StaticBoundObject();

	protected:
		virtual ~StaticBoundObject();

	public:
		/*
			Function: Get

		  Return an object's property. The returned value is automatically
		  reference counted and must be released if the callee does not hold
		  a reference (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name);

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual void GetPropertyNames(std::vector<const char *> *property_names);

		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value);


		/*
			Function: Unset

		  unset the named property
		 */
		virtual void UnSet(const char *name);


		/*
			Function: SetMethod
			
			TODO: Document me
		*/
		template <typename T>
		void SetMethod(const char *name, void (T::*method)(const ValueList&, Value*))
		{
			MethodCallback* callback = NewCallback<T, const ValueList&, Value*>(static_cast<T*>(this), method);

			StaticBoundMethod* bound_method = new StaticBoundMethod(callback);
			Value* method_value = new Value(bound_method);

			this->Set(name, method_value);

			KR_DECREF(bound_method);
			KR_DECREF(method_value);
		}

		/*
			Function: SetObject
			
			TODO: Document me
		*/
		void SetObject(const char *name, BoundObject* object);

		/*
			Function: CreateDelegate

		  create a delegate from a BoundObject to a wrapped
		  StaticBoundObject and delegate set/get to the new
		  static bound object
		 */
		static StaticBoundObject* CreateDelegate(BoundObject *global, BoundObject *bo)
		{
			StaticBoundObject *scope = new StaticBoundObject();
			std::vector<const char *> keys;
			bo->GetPropertyNames(&keys);
			std::vector<const char *>::iterator iter = keys.begin();
			while(iter!=keys.end())
			{
				const char *name = (*iter++);
				std::string key(name);
				Value *value = bo->Get(name);
				if (key == "set")
				{
					ScopeMethodDelegate *d = new ScopeMethodDelegate(SET,global,scope,value->ToMethod());
					Value *v = new Value(d);
					scope->Set(name,v);
					KR_DECREF(d);
					KR_DECREF(v);
				}
				else if (key == "get")
				{
					ScopeMethodDelegate *d = new ScopeMethodDelegate(GET,global,scope,value->ToMethod());
					Value *v = new Value(d);
					scope->Set(name,v);
					KR_DECREF(d);
					KR_DECREF(v);
				}
				else
				{
					scope->Set(name,value);
				}
				KR_DECREF(value);
			}
			return scope;
		}

	protected:
		Mutex mutex;
		std::map<std::string, Value*> properties;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundObject);
	};

}

#endif
