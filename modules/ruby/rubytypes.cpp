/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "rubytypes.h"
#include "rubybinding.h"
#include "rubymodule.h"

static void RubyBind(VALUE self, VALUE name, VALUE object)
{
	if (TYPE(name) == T_STRING) {
		kroll::RubyModule *ruby_module = kroll::RubyModule::Instance();
		VALUE str = rb_inspect(object);
		std::cout << "binding ruby object " <<
			(StringValueCStr(str)) <<
			" as " << (StringValueCStr(name)) <<
			std::endl;

		ruby_module->GetHost()->GetGlobalObject()->BindProperty(
			StringValueCStr(name), new kroll::RubyBinding(object));
	}
}

namespace kroll
{
	const char* RubyTypeToString (int type)
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

	std::string RubyStringToString(VALUE value)
	{
		return std::string(StringValueCStr(value));
	}

	bool RubyBoolToBool(VALUE value)
	{
		if (TYPE(value) == T_TRUE) return true;
		return false;
	}

	int RubyFixnumToInt(VALUE value)
	{
		return (int)NUM2INT(value);
	}

	double RubyFloatToDouble(VALUE value)
	{
		return NUM2DBL(value);
	}

	VALUE wrapper_class;

	static VALUE
	BoundObjectWrapper_MethodMissing(int argc, VALUE *argv, VALUE self) {
		BoundObject* value = NULL;
		Data_Get_Struct(self, BoundObject, value);

		if (value != NULL) {
			VALUE method = rb_funcall(*argv, rb_intern("to_s"), 0);
			method = rb_funcall(method, rb_intern("downcase"), 0);
			const char *method_name = StringValueCStr(method);

			ArgList args;
			for (int i = 1; i < argc; i++) {
				args.push_back(RubyValueToArgValue(argv[i]));
			}
			ReturnValue *returnValue = value->Call(method_name, args);
		}
		return Qnil;
	}

	VALUE BoundObjectWrapper_Create(BoundObject* value)
	{
		VALUE wrapper = Data_Wrap_Struct(wrapper_class, 0, free, value);
		rb_obj_call_init(wrapper, 0, 0);

		return wrapper;
	}

	//VALUE tiClass;
	void BoundObjectWrapper_Init() {
		wrapper_class = rb_define_class("BoundObject", rb_cObject);
		rb_define_method(wrapper_class, "method_missing",
			RUBY_METHOD_FUNC(BoundObjectWrapper_MethodMissing), -1);

		//tiClass = rb_define_class("ti", rb_cObject);
		rb_define_global_function("tiBind", RUBY_METHOD_FUNC(RubyBind), 2);
	}

	ArgValue RubyValueToArgValue(VALUE value)
	{
		switch (TYPE(value)) {
			case T_NIL: return ArgValue();
			case T_DATA: {
				// assume a BoundObject?
				BoundObject* object = NULL;
				Data_Get_Struct(value, BoundObject, object);

				return ArgValue(object);
			} break;
			case T_STRING: return ArgValue(StringValueCStr(value));
			case T_FIXNUM: return ArgValue(NUM2INT(value));
			case T_FLOAT: return ArgValue(NUM2DBL(value));
			case T_TRUE: return ArgValue(true);
			case T_FALSE: return ArgValue(false);
			case T_ARRAY: {
				ArgList list;
				for (int i = 0; i < RARRAY_LEN(value); i++) {
					list.push_back(
						RubyValueToArgValue(rb_ary_entry(value, i)));
				}
				return list;
			} break;
		}
	}

	VALUE ArgListToRubyArray(const ArgList& list)
	{
		VALUE array = rb_ary_new2(list.size());
		ArgList::const_iterator iter;
		for (iter = list.begin(); iter != list.end(); iter++)
		{
			ArgValue value = *iter;
			rb_ary_concat(array, ArgValueToRubyValue(value));
		}
		return array;
	}

	const VALUE* ArgListToCValueArray(const ArgList& list)
	{
		VALUE *cList = new VALUE[list.size()];
		ArgList::const_iterator iter;
		int i = 0;
		for (iter = list.begin(); iter != list.end(); iter++, i++)
		{
			ArgValue value = *iter;
			cList[i] = ArgValueToRubyValue(value);
		}

		return static_cast<const VALUE*>(cList);
	}

	VALUE ArgValueToRubyValue(ArgValue& value)
	{
		if (value.IsBool()) {
			return value.ToBool() ? Qtrue : Qfalse;
		}
		if (value.IsInt()) {
			return INT2NUM(value.ToInt());
		}
		if (value.IsNull()) {
			return Qnil;
		}
		if (value.IsObject()) {
			return BoundObjectWrapper_Create(value.ToObject());
		}
		if (value.IsString()) {
			return rb_str_new2(value.ToString().c_str());
		}
		if (value.IsDouble()) {
			return rb_float_new(value.ToDouble());
		}
		if (value.IsArgList()) {
			return ArgListToRubyArray(value.ToArgList());
		}
		return NULL;
	}
}

