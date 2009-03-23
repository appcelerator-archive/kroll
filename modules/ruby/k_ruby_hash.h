/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_RUBY_HASH_H_
#define _K_RUBY_HASH_H_

namespace kroll {

class KRubyHash : public KObject {
public:
	KRubyHash(VALUE object);
	virtual ~KRubyHash();

	virtual void Set(const char *name, SharedValue value);
	virtual SharedValue Get(const char *name);
	virtual SharedStringList GetPropertyNames();
	virtual SharedString DisplayString(int);
	VALUE ToRuby();

private:
	VALUE hash;
	SharedPtr<KRubyObject> object;

};

}

#endif /* RUBY_BOUND_OBJECT_H_ */
