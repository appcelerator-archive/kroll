/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_SCOPE_METHOD_DELEGATE_H_
#define _KR_BOUND_METHOD_DELEGATE_H_
#include "binding.h"

namespace kroll {
	/*
		Enum: MethodDelegateType
	
		GET - Getter method
		SET - Setter method
	*/
	enum MethodDelegateType
	{
		GET,
		SET
	};
	
	/*
		Class: ScopeMethodDelegate
	
	  class that can be used to change the delegation of a method
	  call's Get or Set method to first check to see if the key has
	  namespace dots (such as ti.foo.bar) and if so, delegate to a
	  differently supplied scope object for delegation.
	 */
	
	
	class KROLL_API ScopeMethodDelegate : public BoundMethod
	{
	public:
		/*
			Constructor: ScopeMethodDelegate
		*/
		ScopeMethodDelegate(MethodDelegateType type, BoundObject *global,
		                    BoundObject *scope, BoundMethod *delegate);

		/*
			Function: Set
	
			TODO: Document me
		*/
		void Set(const char *name, Value* value);

		/*
			Function: Get

			TODO: Document me
		*/
		Value* Get(const char *name);
	
		/*
			Function: GetPropertyNames

			TODO: Document me
		*/
		void GetPropertyNames(std::vector<const char *> *property_names);
	
		/*
			Function: IsGlobalKey
	
			TODO: Document me
		*/
		bool IsGlobalKey(std::string& key);
	
		/*
			Function: Call
	
			TODO: Document me
		*/
		Value* Call(const ValueList& args);

		/*
		  Function: CreateDelegate

		  create a delegate from a BoundObject to a wrapped
		  StaticBoundObject and delegate set/get to the new
		  static bound object
		 */
		static StaticBoundObject* CreateDelegate(BoundObject *global, BoundObject *bo);

	private:
		MethodDelegateType type;
		BoundObject *global;
		BoundObject *scope;
		BoundMethod *delegate;

	protected:
		virtual ~ScopeMethodDelegate();

	private:
		DISALLOW_EVIL_CONSTRUCTORS(ScopeMethodDelegate);
	};

}

#endif
