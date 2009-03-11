/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef RUBY_METHOD_MISSING_H_
#define RUBY_METHOD_MISSING_H_

#include "ruby_module.h"

class RubyMethodMissing : public BoundMethod {
public:
	RubyMethodMissing(VALUE object, std::string& name);
	virtual ~RubyMethodMissing();

	SharedValue Call(const ValueList& args);
	virtual void Set(const char *name, SharedValue value);
	virtual SharedValue Get(const char *name);
	virtual SharedStringList GetPropertyNames();

	VALUE ToRuby() { return rb_funcall(object, rb_intern("method"), 1, ID2SYM(rb_intern("method_missing"))); }

private:
	VALUE object;
	std::string method_name;
};

#endif /* RUBY_METHOD_MISSING_H_ */
