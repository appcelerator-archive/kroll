/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_RUBY_METHOD_H_
#define _K_RUBY_METHOD_H_

namespace kroll {

class KRubyMethod : public KMethod
{
	public:
	KRubyMethod(VALUE method);
	KRubyMethod(VALUE method, VALUE arg);
	virtual ~KRubyMethod();
	SharedValue Call(const ValueList& args);
	virtual void Set(const char *name, SharedValue value);
	virtual SharedValue Get(const char *name);
	virtual SharedStringList GetPropertyNames();
	virtual SharedString DisplayString(int);
	VALUE ToRuby();

	private:
	VALUE method;
	VALUE arg;
	SharedPtr<KRubyObject> object;
};

}

#endif
