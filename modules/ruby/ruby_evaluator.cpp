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
		bool assignment = false;

		if (global_object.isNull())
			return Qnil;

		// store the method name and arguments in separate variables
		VALUE method_name, args;
		rb_scan_args(argc, argv, "1*", &method_name, &args);
		char* name = strdup(rb_id2name(SYM2ID(method_name)));

		// Check if this is an assignment
		if (name[strlen(name) - 1] == '=')
		{
			name[strlen(name) - 1] = '\0';
			assignment = true;
		}
		// If we can't find this property perhaps we should return
		// the same property name except capitalized.
		SharedValue v = global_object->Get(name);
		if (v->IsUndefined())
		{
			name[0] = toupper(name[0]);
			v = global_object->Get(name);
		}
		// Okay, maybe not
		if (v->IsUndefined())
			name[0] = tolower(name[0]);

		VALUE rval;
		if (assignment) // Assignment
		{
			rval = rb_ary_entry(args, 0);
			SharedValue val = RubyUtils::ToKrollValue(rval);
			global_object->Set(name, val);
		}
		if (v->IsMethod()) // Method call
		{
			rval = RubyUtils::GenericKMethodCall(v->ToMethod(), args);
		}
		else // Plain old access
		{
			rval = RubyUtils::ToRubyValue(v);
		}
		return rval;
	}

	std::string RubyEvaluator::GetContextId(SharedKObject global)
	{
		int cid = 0;
		SharedValue idv = global->Get("__ruby_module_id__");
		if (idv->IsUndefined())
		{
			cid = this->next_id++;
			global->Set("__ruby_module_id__", Value::NewInt(cid));
		}
		else
		{
			cid = idv->ToInt();
		}
		return std::string("$windowProc") + KList::IntToChars(cid);
	}

	VALUE RubyEvaluator::GetContext(SharedKObject global)
	{
		std::string theid = this->GetContextId(global);
		VALUE ctx = rb_gv_get(theid.c_str());
		if (ctx == Qnil)
		{
			VALUE ctx_class = rb_define_class("KrollRubyContext", rb_cObject);
			rb_define_method(ctx_class, "method_missing", VALUEFUNC(m_missing), -1); 
			ctx = rb_obj_alloc(ctx_class);
			rb_gv_set(theid.c_str(), ctx);
		}
		return ctx;
	}

	VALUE reval_do_call(VALUE args)
	{
		// Don't use rb_obj_instance_eval here, as it will implicitly
		// use any Ruby code block that was passed. See:
		// http://banisterfiend.wordpress.com/2008/09/25/metaprogramming-in-the-ruby-c-api-part-one-blocks/
		VALUE ctx = rb_ary_shift(args);
		VALUE code = rb_ary_shift(args);
		return rb_funcall(ctx, rb_intern("instance_eval"), 1, code);
	}

	SharedValue RubyEvaluator::Call(const ValueList& args)
	{
		if (args.size() != 3
			|| !args.at(1)->IsString()
			|| !args.at(2)->IsObject())
			return Value::Undefined;
		const char* code = args.at(1)->ToString();
		global_object = args.at(2)->ToObject();
		VALUE ctx = this->GetContext(global_object);

		VALUE rargs = rb_ary_new();
		rb_ary_push(rargs, ctx);
		rb_ary_push(rargs, rb_str_new2(code));

		int error;
		VALUE result = rb_protect(reval_do_call, rargs, &error);
		RubyEvaluator::ContextToGlobal(ctx, global_object);

		if (error != 0)
		{
			// TODO: Logging
			ValueException e = RubyUtils::GetException();
			SharedString ss = e.DisplayString();
			std::cout << "An error occured while parsing "
			          << "Ruby on the page: " << std::endl;
			
			std::cout << *ss << std::endl;
			return Value::Undefined;
		}

		return RubyUtils::ToKrollValue(result);
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
