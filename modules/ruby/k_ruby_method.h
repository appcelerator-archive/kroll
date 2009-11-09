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
	KRubyMethod(VALUE method, const char*);
	KRubyMethod(VALUE method, VALUE arg);
	virtual ~KRubyMethod();
	KValueRef Call(const ValueList& args);
	virtual void Set(const char *name, KValueRef value);
	virtual KValueRef Get(const char *name);
	virtual SharedStringList GetPropertyNames();
	virtual SharedString DisplayString(int);
	VALUE ToRuby();

	/*
	 * Determine if the given Ruby object equals this one
	 * by comparing these objects's identity e.g. equals?()
	 *  @param other the object to test
	 *  @returns true if objects have reference equality, false otherwise
	 */
	virtual bool Equals(KObjectRef);

	private:
	VALUE method;
	VALUE arg;
	AutoPtr<KRubyObject> object;
	char* name;
};

}

#endif
