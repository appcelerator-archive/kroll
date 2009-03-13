/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#include "k_ruby_object.h"
#include "k_ruby_method.h"
#include "ruby_method_missing.h"

namespace kroll
{
	static VALUE ruby_wrapper_class = NULL;
	SharedPtr<BoundObject> RubyUtils::scope;

	const char * RubyUtils::ToUpper (const char * c)
	{
		std::string s(c);
		std::transform(s.begin(), s.end(), s.begin(), toupper);

		return strdup(s.c_str());
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

	const char * RubyUtils::ToString(VALUE value)
	{
		if (TYPE(value)==T_STRING)
		{
			const char *result = StringValueCStr(value);
			if (result!=NULL)
			{
				return result;
			}
		}
		return NULL;
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

	SharedBoundMethod RubyUtils::ToMethod(VALUE value)
	{
		return new KRubyMethod(value);
	}

	SharedBoundObject RubyUtils::ToObject(VALUE value)
	{
		return new KRubyObject(value);
	}

	SharedBoundMethod RubyUtils::CreateMethodMissing(VALUE object, std::string& method_name)
	{
		return new RubyMethodMissing(object, method_name);
	}

	SharedValue RubyUtils::ToKrollValue(VALUE value)
	{
		switch (TYPE(value)) {
			case T_NIL: return Value::Null;
			case T_DATA:
			{
				VALUE klass = rb_obj_class(value);
				if (klass == ruby_wrapper_class) {
					SharedPtr<BoundObject> *object = NULL;
					Data_Get_Struct(value, SharedPtr<BoundObject>, object);
					// is this a method ?
					if (typeid(object->get()) == typeid(BoundMethod*)) {
						SharedPtr<BoundMethod> method = object->cast<BoundMethod>();
						return Value::NewMethod(method);
					}
					// is this a list ?
					if (typeid(object->get()) == typeid(BoundList*)) {
						SharedPtr<BoundList> list = object->cast<BoundList>();
						return Value::NewList(list);
					}

					// nope, it's just a plain 'ole object
					return Value::NewObject(object->get());
				}
				else {
					if (rb_funcall(value, rb_intern("is_a?"), 1, rb_path2class("Method"))) {
						SharedBoundMethod method = RubyUtils::ToMethod(value);
						return Value::NewMethod(method);
					}
					else {
						SharedBoundObject object = RubyUtils::ToObject(value);
						return Value::NewObject(object);
					}
				}
			}
			break;
			case T_STRING: return Value::NewString(StringValueCStr(value));
			case T_FIXNUM: return Value::NewInt(NUM2INT(value));
			case T_FLOAT: return Value::NewDouble(NUM2DBL(value));
			case T_TRUE: return Value::NewBool(true);
			case T_FALSE: return Value::NewBool(false);
			case T_ARRAY:
			{
				SharedPtr<BoundList> list = new StaticBoundList();
				//ScopedDereferencer r(list);
				for (int i = 0; i < RARRAY_LEN(value); i++)
				{
					SharedValue arg = ToKrollValue(rb_ary_entry(value, i));
					list->Append(arg);
					//KR_DECREF(arg);
				}
				return Value::NewList(list);
			} break;
		}
		return Value::Undefined;
	}

	VALUE RubyUtils::Create(SharedPtr<BoundList> list)
	{
		VALUE array = rb_ary_new2(list->Size());
		for (unsigned int c=0; c < list->Size(); c++)
		{
			SharedValue value = list->At(c);
			VALUE arg = ToRubyValue(value);
			rb_ary_concat(array, arg);
			//KR_DECREF(value);
		}
		return array;
	}

	VALUE RubyUtils::ToRubyValue(SharedValue value)
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
		if (value->IsObject() || value->IsMethod()) {
			return Create(value->ToObject());
		}
		if (value->IsList()) {
			return Create(value->ToList());
		}
		if (value->IsString()) {
			return rb_str_new2(value->ToString());
		}
		if (value->IsDouble()) {
			return rb_float_new(value->ToDouble());
		}
		//docs say we must never return NULL
		return Qnil;
	}

	static void KRubyObjectFree(void *p)
	{
		SharedPtr<BoundObject> *obj = static_cast< SharedPtr<BoundObject>* >(p);
		//KR_DECREF(obj);
		delete obj;
	}

	VALUE RubyUtils::Create(SharedPtr<BoundObject> value)
	{
		VALUE wrapper = Data_Wrap_Struct(ruby_wrapper_class, 0, KRubyObjectFree, new SharedPtr<BoundObject>(value));
		rb_obj_call_init(wrapper, 0, 0);
		//KR_ADDREF(value);
		return wrapper;
	}

	ValueException RubyUtils::GetException(int error)
	{
		if(error == 0)
			return ValueException::FromString("Unknown");

		VALUE e = rb_gv_get("$!");
		SharedValue v = RubyUtils::ToKrollValue(e);
		return ValueException(v);
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
			ValueException e = RubyUtils::GetException(error);
			throw e;
		}
		return result;
	}

	static const char* ToMethodName(VALUE value)
	{
		VALUE method = RubyFuncCall(value, rb_intern("to_s"));
		//method = RubyFuncCall(method, rb_intern("downcase"));
		return StringValueCStr(method);
	}

	static VALUE KRubyObjectMethodDefined(int argc, VALUE *argv, VALUE self)
	{
		SharedPtr<BoundObject> *object = NULL;
		Data_Get_Struct(self, SharedPtr<BoundObject>, object);

		if (object != NULL)
		{
			const char *name = ToMethodName(*argv);
			SharedValue value = (*object)->Get(name);
			return value->IsMethod() ? Qtrue : Qfalse;
		}
		return Qfalse;
	}

	static VALUE KRubyObjectMethodMissing(int argc, VALUE *argv, VALUE self)
	{
		SharedPtr<BoundObject> *object = NULL;
		Data_Get_Struct(self, SharedPtr<BoundObject>, object);

		if (object != NULL)
		{
			// we need to determine the method that was invoked
			const char *method_name = ToMethodName(*argv);

			// is this a setter (property=)
			bool setter = (method_name[strlen(method_name)-1]) == '=';

			SharedValue value = NULL;

			if (setter)
			{
				char *s = strdup(method_name);
				s[strlen(method_name)-1]='\0'; // trim the =
				SharedValue arg = RubyUtils::ToKrollValue(argv[1]);
				(*object)->Set(s,arg);
				free(s);
				//KR_DECREF(arg);
				return Qnil;
			}
			else
			{
			 	value = (*object)->Get(method_name);
			}

			// method we can invoke
			if (value->IsMethod())
			{
				ValueList args;
				// convert the passed in arguments to
				// Value* objects suitable for passing to Call
				for (int i = 1; i < argc; i++)
				{
					SharedValue arg = RubyUtils::ToKrollValue(argv[i]);
					args.push_back(arg);
				}
				// convert and invoke
				SharedValue result = value->ToMethod()->Call(args);
				//ScopedDereferencer r(result);
				// free the args before we return
				// -- these will be freed by convention when the vector is freed
				/*ValueList::iterator i = args.begin();
				while(i!=args.end())
				{
					Value *a = (*i++);
					KR_DECREF(a);
				}*/
				return RubyUtils::ToRubyValue(result);
			}
			else
			{
				// this is a getter
				return RubyUtils::ToRubyValue(value);
			}
		}
		return Qnil;
	}

	void RubyUtils::InitializeDefaultBindings(Host *host)
	{
		ruby_wrapper_class = rb_define_class("KRubyObject", rb_cObject);

		// overide the method_missing magic function that's called
		// for any access to our object (essentially)
		rb_define_method(ruby_wrapper_class, "method_missing",
			RUBY_METHOD_FUNC(KRubyObjectMethodMissing), -1);

		// override method_defined? to support reflection of
		// methods to determine if they are defined on an object
		rb_define_method(ruby_wrapper_class, "method_defined?",
			RUBY_METHOD_FUNC(KRubyObjectMethodDefined), -1);

		//// now bind our new scope to python module
		//SharedBoundObject rubyObject = new StaticBoundObject();
		//SharedValue scopeRef = Value::NewObject(rubyObject);
		//host->GetGlobalObject()->Set((const char*)"Ruby",scopeRef);
		//host->GetGlobalObject()->SetNS("Ruby.evaluate", Value::NewMethod(evaluator));
		//// don't release the scope
	}
}


