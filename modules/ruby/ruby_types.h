/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef RUBY_TYPES_H_
#define RUBY_TYPES_H_

#include <api/binding/bound_object.h>
#include <ruby.h>

namespace kroll
{
	void Bind(const char *name, VALUE ruby_object);

	ArgValue RubyValueToArgValue(VALUE value);
	VALUE ArgValueToRubyValue(ArgValue& value);
	VALUE ArgListToRubyArray(const ArgList& list);
	const VALUE* ArgListToCValueArray(const ArgList& list);

	void BoundObjectWrapper_Init();
	VALUE BoundObjectWrapper_Create(BoundObject* value);
}

#endif
