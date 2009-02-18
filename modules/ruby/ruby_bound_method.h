/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef RUBY_BOUND_METHOD_H_
#define RUBY_BOUND_METHOD_H_

#include "ruby_module.h"

namespace kroll {

class RubyBoundMethod : public BoundMethod {
public:
	RubyBoundMethod(VALUE method);
	virtual ~RubyBoundMethod();
	SharedValue Call(const ValueList& args);
	virtual void Set(const char *name, SharedValue value);
	virtual SharedValue Get(const char *name);
	virtual SharedStringList GetPropertyNames();

	VALUE ToRuby() { return method; }
private:
	VALUE method;
	SharedBoundObject delegate;
};

}

#endif /* RUBY_BOUND_METHOD_H_ */
