/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_RUBY_OBJECT_H_
#define _K_RUBY_OBJECT_H_

#include "ruby_module.h"

namespace kroll {

class KRubyObject : public KObject {
public:
	KRubyObject(VALUE object);
	virtual ~KRubyObject();

	virtual void Set(const char *name, SharedValue value);
	virtual SharedValue Get(const char *name);
	virtual SharedStringList GetPropertyNames();

	const VALUE ToRuby() { return object; }

private:
	VALUE object;

};

}

#endif /* RUBY_BOUND_OBJECT_H_ */
