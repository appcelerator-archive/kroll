/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"
#include <string>

namespace kroll
{
	static VALUE ruby_wrapper_class = NULL;
	
	std::string RubyUtils::ToUpper (std::string s) 
	{
		std::string copy;
		std::string::iterator i = s.begin();
		while(i!=s.end())
		{
			copy+=std::toupper((*i++));
		}
		return copy;
	}
	const char* RubyUtils::TypeToString (int type)
	{
	  switch (type) {
	    case T_NIL: return "T_NIL";
	    case T_STRING: return "T_STRING";
	    case T_TRUE: return "T_TRUE";
	    case T_FALSE: return "T_FALSE";
	    case T_FIXNUM: return "T_FIXNUM";
	    case T_FLOAT: return "T_FLOAT";
	  }
	  return "UNKNOWN??";
	}

	std::string RubyUtils::ToString(VALUE value)
	{
		if (TYPE(value)==T_STRING)
		{
			const char *result = StringValueCStr(value);
			if (result!=NULL)
			{
				return std::string(result);
			}
		}
		return std::string();
	}

	bool RubyUtils::ToBool(VALUE value)
	{
		return TYPE(value) == T_TRUE;
	}

	int RubyUtils::ToInt(VALUE value)
	{
		return (int)NUM2INT(value);
	}

	double RubyUtils::ToDouble(VALUE value)
	{
		return NUM2DBL(value);
	}
	
	Value* RubyUtils::ToValue(VALUE value)
	{
		switch (TYPE(value)) {
			case T_NIL: return Value::Null();
			case T_DATA: 
			{
				//TODO: test to see if its a function
				
				BoundObject* object = NULL;
				Data_Get_Struct(value, BoundObject, object);
				return new Value(object);
			} 
			break;
			case T_STRING: return new Value(StringValueCStr(value));
			case T_FIXNUM: return new Value(NUM2INT(value));
			case T_FLOAT: return new Value(NUM2DBL(value));
			case T_TRUE: return new Value(true);
			case T_FALSE: return new Value(false);
			case T_ARRAY: 
			{
				BoundList* list = new StaticBoundList(); 
				ScopedDereferencer r(list);
				for (int i = 0; i < RARRAY_LEN(value); i++) 
				{
					Value *arg = ToValue(rb_ary_entry(value, i));
					list->Append(arg);
					KR_DECREF(arg);
				}
				return new Value(list);
			} break;
		}
		return Value::Undefined();
	}

	// VALUE RubyUtils::ToArray(BoundList* list)
	// {
	// 	// VALUE array = rb_ary_new2(list.size());
	// 	// ArgList::const_iterator iter;
	// 	// for (iter = list.begin(); iter != list.end(); iter++)
	// 	// {
	// 	// 	ArgValue value = *iter;
	// 	// 	rb_ary_concat(array, ArgValueToRubyValue(value));
	// 	// }
	// 	// return array;
	// 	return Qnil;
	// }

	const VALUE* RubyUtils::FromArray(BoundList* list)
	{
		VALUE *result = new VALUE[list->Size()];
		for (int c=0;c<list->Size();c++)
		{
			result[c] = ToValue(list->At(c));
		}
		return static_cast<const VALUE*>(result);
	}

	VALUE RubyUtils::ToValue(Value* value)
	{
		//TODO: undefined
		if (value->IsBool()) {
			return value->ToBool() ? Qtrue : Qfalse;
		}
		if (value->IsInt()) {
			return INT2NUM(value->ToInt());
		}
		if (value->IsNull()) {
			return Qnil;
		}
		if (value->IsObject() || value->IsMethod()) 
		{
			return Create(value->ToObject());
		}
		if (value->IsString()) {
			return rb_str_new2(value->ToString().c_str());
		}
		if (value->IsDouble()) {
			return rb_float_new(value->ToDouble());
		}
		if (value->IsList()) {
			//return ArgListToRubyArray(value.ToArgList());
		}
		return NULL;
	}

	static void RubyBoundObjectFree(void *p)
	{
		BoundObject *obj = static_cast<BoundObject*>(p);
		KR_DECREF(obj);
	}

	VALUE RubyUtils::Create(BoundObject* value)
	{
		VALUE wrapper = Data_Wrap_Struct(ruby_wrapper_class, 0, RubyBoundObjectFree, value);
		rb_obj_call_init(wrapper, 0, 0);
		KR_ADDREF(value);
		return wrapper;
	}	

	static VALUE RubyBoundObjectMethodMissing(int argc, VALUE *argv, VALUE self) 
	{
		BoundObject* v = NULL;
		Data_Get_Struct(self, BoundObject, v);
		
		if (v != NULL) 
		{
			// we need to determine the method that was invoked
			VALUE method = rb_funcall(*argv, rb_intern("to_s"), 0);

			// TODO: do we need to downcase?
			method = rb_funcall(method, rb_intern("downcase"), 0);
			const char *method_name = StringValueCStr(method);
			
			// look up our property to make sure it's a 
			// method we can invoke
			Value *m = v->Get(method_name);
			if (m->IsMethod())
			{
				ValueList args;
				// convert the passed in arguments to 
				// Value* objects suitable for passing to Call
				for (int i = 1; i < argc; i++) 
				{
					Value *arg = RubyUtils::ToValue(argv[i]);
					args.push_back(arg);
				}
				// convert and invoke
				Value *result = m->ToMethod()->Call(args);
				ScopedDereferencer r(result);
				// free the args before we return
				ValueList::iterator i = args.begin();
				while(i!=args.end())
				{
					Value *a = (*i++);
					KR_DECREF(a);
				}
				return RubyUtils::ToValue(result);
			}
		}
		return Qnil;
	}
	
	void RubyUtils::InitializeDefaultBindings(Host *host)
	{
		ruby_wrapper_class = rb_define_class("BoundObject", rb_cObject);
		
		rb_define_method(ruby_wrapper_class, "method_missing",
			RUBY_METHOD_FUNC(RubyBoundObjectMethodMissing), -1);
		
		
		// we bind the special module "api" to the global
		// variable defined in PRODUCT_NAME to give the
		// Python runtime access to it
		Value *api = host->GetGlobalObject()->Get("api");
		if (api->IsObject())
		{
			// we're going to clone the methods from api into our
			// own python scoped object
			StaticBoundObject *scope = ScopeMethodDelegate::CreateDelegate(host->GetGlobalObject(),api->ToObject());
			
			// convert our static global guy to a Ruby VALUE
			// and then make it a global const
			VALUE scope_value = Create(scope);
			
			// Ruby global constants must be ALL UPPERCASE
			rb_define_global_const(ToUpper(std::string(PRODUCT_NAME)).c_str(),scope_value);

			// now bind our new scope to python module
			Value *scopeRef = new Value(scope);
			host->GetGlobalObject()->Set((const char*)"ruby",scopeRef);
			KR_DECREF(scopeRef);
			// don't release the scope
		}
	}
}


