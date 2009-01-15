/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"
#include <string>
#include <sstream>
#include <stdexcept>

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
				BoundObject* object = NULL;
				Data_Get_Struct(value, BoundObject, object);
				// is this a method ?
				BoundMethod *method = dynamic_cast<BoundMethod*>(object);
				if (method!=NULL)
				{
					return new Value(method);
				}
				// is this a list ?
				BoundList *list = dynamic_cast<BoundList*>(object);
				if (list!=NULL)
				{
					return new Value(list);
				}
				// nope, it's just a plain 'ole object 
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

	VALUE RubyUtils::Create(BoundList* list)
	{
		VALUE array = rb_ary_new2(list->Size());
		for (int c=0;c<list->Size();c++)
		{
			Value* value = list->At(c);
			VALUE arg = ToValue(value);
			rb_ary_concat(array, arg);
			KR_DECREF(value);
		}
		return array;
	}

	VALUE RubyUtils::ToValue(Value* value)
	{
		if (value->IsBool()) {
			return value->ToBool() ? Qtrue : Qfalse;
		}
		if (value->IsInt()) {
			return INT2NUM(value->ToInt());
		}
		if (value->IsNull() || value->IsUndefined()) {
			return Qnil;
		}
		if (value->IsObject()) {
			return Create(value->ToObject());
		}
		if (value->IsMethod()) {
			return Create(value->ToMethod());
		}
		if (value->IsList()) {
			return Create(value->ToList());
		}
		if (value->IsString()) {
			return rb_str_new2(value->ToString().c_str());
		}
		if (value->IsDouble()) {
			return rb_float_new(value->ToDouble());
		}
		//docs say we must never return NULL
		return Qnil; 
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
	
	// adopted from http://metaeditor.sourceforge.net/embed/#id2841270
	static void ThrowOnError(int error) 
	{
	    if(error == 0)
	        return;

	    VALUE lasterr = rb_gv_get("$!");

	    // message
	    VALUE message = rb_obj_as_string(lasterr);
		const char *exception = RSTRING(message)->ptr;

	    // backtrace
	    if(!NIL_P(ruby_errinfo)) {
	        std::ostringstream o;
	        VALUE ary = rb_funcall(
	            ruby_errinfo, rb_intern("backtrace"), 0);
	        int c;
			o << "Exception: " << exception << "\n";
	        for (c=0; c<RARRAY(ary)->len; c++) {
	            o << "\tfrom " << 
	                RSTRING(RARRAY(ary)->ptr[c])->ptr << 
	                "\n";
	        }
			std::cerr << o.str() << std::endl;
	    }
	    throw std::runtime_error(exception);
	}
	
	static VALUE RubySafeFuncCall(VALUE args)
	{
		VALUE *a = (VALUE*)args;
 		return rb_funcall(a[0],a[1],0,0);
	}

	static VALUE RubyFuncCall(VALUE self, VALUE name)
	{ 
		int error = 0;
		//NOTE: for now, we're invoking only methods that
		//have no args - this won't work when we need args
		VALUE args[2];
		args[0] = self;
		args[1] = name;
		VALUE result = rb_protect(RubySafeFuncCall, (VALUE)&args, &error);
		if (error != 0) 
		{ 
		    ThrowOnError(error); 
		    result = Qnil; 
		} 
		return result;
	}
	
	static const char* ToMethodName(VALUE value)
	{
		VALUE method = RubyFuncCall(value, rb_intern("to_s"));
		method = RubyFuncCall(method, rb_intern("downcase"));
		return StringValueCStr(method);
	}

	static VALUE RubyBoundObjectMethodDefined(int argc, VALUE *argv, VALUE self) 
	{
		BoundObject* object = NULL;
		Data_Get_Struct(self, BoundObject, object);
		
		if (object != NULL) 
		{
			const char *name = ToMethodName(*argv);
			Value *value = object->Get(name);
			return value->IsMethod() ? Qtrue : Qfalse;
		}
		return Qfalse;
	}

	static VALUE RubyBoundObjectMethodMissing(int argc, VALUE *argv, VALUE self) 
	{
		BoundObject* object = NULL;
		Data_Get_Struct(self, BoundObject, object);
		
		if (object != NULL) 
		{
			// we need to determine the method that was invoked
			const char *method_name = ToMethodName(*argv);
			
			// is this a setter (property=)
			bool setter = (method_name[strlen(method_name)-1]) == '=';
			
			Value *value = NULL;
			
			if (setter)
			{
				char *s = strdup(method_name);
				s[strlen(method_name)-1]='\0'; // trim the =
				Value *arg = RubyUtils::ToValue(argv[1]);
				object->Set(s,arg);
				free(s);
				KR_DECREF(arg);
				return Qnil;
			}
			else
			{
			 	value = object->Get(method_name);
			}
			
			// method we can invoke
			if (value->IsMethod())
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
				Value *result = value->ToMethod()->Call(args);
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
			else
			{
				// this is a getter
				return RubyUtils::ToValue(value);
			}
		}
		return Qnil;
	}
	
	void RubyUtils::InitializeDefaultBindings(Host *host)
	{
		ruby_wrapper_class = rb_define_class("RubyBoundObject", rb_cObject);

		// overide the method_missing magic function that's called
		// for any access to our object (essentially)
		rb_define_method(ruby_wrapper_class, "method_missing",
			RUBY_METHOD_FUNC(RubyBoundObjectMethodMissing), -1);

		// override method_defined? to support reflection of 
		// methods to determine if they are defined on an object
		rb_define_method(ruby_wrapper_class, "method_defined?",
			RUBY_METHOD_FUNC(RubyBoundObjectMethodDefined), -1);
		
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


