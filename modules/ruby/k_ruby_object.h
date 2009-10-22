/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_RUBY_OBJECT_H_
#define _K_RUBY_OBJECT_H_

namespace kroll {

class KRubyObject : public KObject {
public:
	KRubyObject(VALUE object);
	virtual ~KRubyObject();

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
	VALUE object;

};

}

#endif /* RUBY_BOUND_OBJECT_H_ */
