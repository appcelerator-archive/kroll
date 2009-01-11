/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_METHOD_H_
#define _KR_BOUND_METHOD_H_

#include "binding.h"
#include <cstdarg>

namespace kroll
{
	typedef Callback2<const ValueList&, Value *>::Type MethodCallback;


	class KROLL_API BoundMethod : public BoundObject
	{
	protected:
		virtual ~BoundMethod(){}
	public:
		BoundMethod() {}
		/**
		 * invoke the bound method. the returned value is automatically
		 * reference counted and you must release the reference when finished
		 * with the return value (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Call(const ValueList& args) = 0;

		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference,
		 * if they increase the reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value) = 0;

		/**
		 * return a named property. the returned value is automatically
		 * reference counted and you must release the reference when finished
		 * with the return value (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name) = 0;

		/**
		 * Return a list of this object's property names.
		 */
		virtual void GetPropertyNames(std::vector<std::string> *property_names) = 0;

		/**
		 * call the method with a variable list of Value* arguments
		 * When an error occurs will throw an exception of type Value*.
		 */
		Value* Call(Value *first, ...)
		{
			ValueList args;
			va_list vaargs;
			va_start(vaargs,first);
			args.push_back(first);
			while(1)
			{
		      Value* a = va_arg(vaargs,Value*);
		      if (a==NULL) break;
		      args.push_back(a);
			}
			va_end(vaargs);
			return this->Call(args);
		}
		//NOTE: this ideally above would be an operator() overload
		//so you could just method() invoke this function but it doesn't
		//compile on GCC with "cannot be used as function"

	private:
		DISALLOW_EVIL_CONSTRUCTORS(BoundMethod);
	};

	enum MethodDelegateType
	{
		GET,
		SET
	};
	/**
	 * class that can be used to change the delegation of a method
	 * call's Get or Set method to first check to see if the key has
	 * namespace dots (such as ti.foo.bar) and if so, delegate to a
	 * differently supplied scope object for delegation.
	 */
	class ScopeMethodDelegate : public BoundMethod
	{
	public:
		ScopeMethodDelegate(MethodDelegateType type, BoundObject *global, BoundObject *scope, BoundMethod *delegate) :
			type(type),global(global),scope(scope),delegate(delegate)
		{
			KR_ADDREF(global);
			KR_ADDREF(scope);
			KR_ADDREF(delegate);
		}
		void Set(const char *name, Value* value)
		{
			delegate->Set(name,value);
		}
		Value* Get(const char *name)
		{
			return delegate->Get(name);
		}
		void GetPropertyNames(std::vector<std::string> *property_names)
		{
			delegate->GetPropertyNames(property_names);
		}
		bool IsGlobalKey(std::string& key)
		{
			std::string::size_type pos = key.find_first_of(".");
			return (pos!=std::string::npos);
		}
		Value* Call(const ValueList& args)
		{
			std::string key = args.at(0)->ToString();
			BoundObject* obj = IsGlobalKey(key) ? global : scope;
			if (type == GET)
			{
				// not found, look inside scope
				return obj->Get(key.c_str());
			}
			else
			{
				obj->SetNS(key.c_str(),args.at(1));
				return Value::Undefined();
			}
		}
	private:
		MethodDelegateType type;
		BoundObject *global;
		BoundObject *scope;
		BoundMethod *delegate;
	protected:
		virtual ~ScopeMethodDelegate()
		{
			KR_DECREF(global);
			KR_DECREF(scope);
			KR_DECREF(delegate);
		}
	};

}


#endif

