/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ruby_module.h"
#include <cstring>
#include <iostream>
#include <sstream>

namespace kroll
{

	RubyEvaluator::RubyEvaluator() :
		next_id(0)
	{
	}

	RubyEvaluator::~RubyEvaluator()
	{
	}

	static SharedKObject global_object;
	static VALUE m_missing(int argc, VALUE* argv, VALUE self)
	{
		if (global_object.isNull())
			return Qnil;

		// store the method name and arguments in separate variables
		VALUE method_name, args;
		rb_scan_args(argc, argv, "1*", &method_name, &args);

		const char* name = rb_id2name(SYM2ID(method_name));

		// This is an assignment
		if (name[strlen(name) - 1] == '=')
		{
			char* chopped_name = strdup(name);
			chopped_name[strlen(name) - 1] = '\0';
			VALUE rval = rb_ary_entry(args, 0);
			SharedValue val = RubyUtils::ToKrollValue(rval);
			global_object->Set(chopped_name, val);
			return rval;
		}

		// This is an access
		SharedValue v = global_object->Get(name);
		if (v->IsMethod()) // It's a method, call it
		{
			try
			{
				ValueList kargs;
				for (int i = 0; i < RARRAY(args)->len; i++)
				{
					VALUE rarg = rb_ary_entry(args, i);
					SharedValue arg = RubyUtils::ToKrollValue(rarg);
					kargs.push_back(arg);
				}
				v = v->ToMethod()->Call(kargs);
			}
			catch (ValueException& e)
			{
				VALUE rex = RubyUtils::ToRubyValue(e.GetValue());
				rb_raise(rex, "Kroll exception");
			}
		}

		VALUE rv = RubyUtils::ToRubyValue(v);
		return rv;
	}

	std::string RubyEvaluator::GetContextId(SharedKObject global)
	{
		int id = 0;
		SharedValue idv = global->Get("__ruby_module_id__");
		if (idv->IsUndefined())
		{
			id = this->next_id++;
			global->Set("__ruby_module_id__", Value::NewInt(id));
		}
		else
		{
			id = idv->ToInt();
		}
		return std::string("$windowProc") + KList::IntToChars(id);
	}

	VALUE RubyEvaluator::GetContext(SharedKObject global)
	{
		std::string id = this->GetContextId(global);
		VALUE ctx = rb_gv_get(id.c_str());
		if (ctx == Qnil)
		{
			VALUE ctx_class = rb_define_class("KrollRubyContext", rb_cObject);
			rb_define_method(ctx_class, "method_missing", VALUEFUNC(m_missing), -1); 
			ctx = rb_obj_alloc(ctx_class);
			rb_gv_set(id.c_str(), ctx);
		}
		return ctx;
	}

	VALUE reval_do_call(VALUE args)
	{
		VALUE ctx = rb_ary_shift(args);
		VALUE code = rb_ary_shift(args);
		return rb_funcall(ctx, rb_intern("instance_eval"), 1, code);
	}
	VALUE reval_handle_exception(VALUE args)
	{
		ValueException e = RubyUtils::GetException(1);
		SharedString ss = e.DisplayString();
		std::cout << "An error occured while parsing "
		          << "Ruby on the page: " << std::endl;
		
		std::cout << *ss << std::endl;
		return Qnil;
	}
	SharedValue RubyEvaluator::Call(const ValueList& args)
	{
		try
		{
			if (args.size() != 3
				|| !args.at(1)->IsString()
				|| !args.at(2)->IsObject())
				return Value::Undefined;
			const char* code = args.at(1)->ToString();
			global_object = args.at(2)->ToObject();
			VALUE ctx = this->GetContext(global_object);

			// Push new methods into the window global scope.
			VALUE rargs = rb_ary_new();
			rb_ary_push(rargs, ctx);
			rb_ary_push(rargs, rb_str_new2(code));
			VALUE ret_val = rb_rescue(
				VALUEFUNC(reval_do_call), rargs,
				VALUEFUNC(reval_handle_exception), rargs);

			RubyEvaluator::ContextToGlobal(ctx, global_object);
			return RubyUtils::ToKrollValue(ret_val);

		}
		catch (ValueException& e)
		{
			// While Ruby exceptions will be caputured by
			// rb_rescue, exceptions from the Kroll layer
			// will have to be caught here.
			SharedString ss = e.DisplayString();
			std::cout << "An error occured while parsing "
			          << "Ruby on the page: " << std::endl;
			
			std::cout << *ss << std::endl;
			return Value::Undefined;
		}
	}

	void RubyEvaluator::ContextToGlobal(VALUE ctx, SharedKObject o)
	{
		if (global_object.isNull())
			return;

		// Next copy all methods over -- they override variables
		VALUE methods = rb_funcall(ctx, rb_intern("singleton_methods"), 0);
		for (long i = 0; i < RARRAY(methods)->len; i++)
		{
			VALUE meth_symbol = rb_ary_entry(methods, i);
			const char* meth_name = STR2CSTR(meth_symbol);

			// Skip our special method_missing method
			if (strcmp(meth_name, "method_missing") == 0)
				continue;

			VALUE rmeth = rb_funcall(ctx, rb_intern("method"), 1, meth_symbol);
			SharedValue meth = Value::NewMethod(new KRubyMethod(rmeth));
			o->Set(meth_name, meth);
		}
	}

	void RubyEvaluator::Set(const char *name, SharedValue value)
	{
	}

	SharedValue RubyEvaluator::Get(const char *name)
	{
		return Value::Undefined;
	}

	SharedStringList RubyEvaluator::GetPropertyNames()
	{
		return SharedStringList();
	}
}
