/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_OBJECT_H_
#define _KR_ACCESSOR_BOUND_OBJECT_H_

namespace kroll
{
	/**
	 * AccessorBoundObjects automatically expose any property as a getter or setter
	 * For example, if you expose a property "length" via:
	 * <code>
	 * this->Set("length", lengthValue);
	 * </code>
	 *
	 * A caller could then call "getLength" or "setLength", as well as accessing the property directly.
	 */
	class KROLL_API AccessorBoundObject : public StaticBoundObject
	{
	public:
		AccessorBoundObject();

		/**
		 * @see KObject::Set
		 */
		virtual void Set(const char *name, SharedValue value);

		/**
		 * @see KObject::Get
		 */
		virtual SharedValue Get(const char *name);

		SharedValue RawGet(const char *name);
		void RawSet(const char *name, SharedValue value);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(AccessorBoundObject);
		static std::string Capitalize(const char *word);
	};
}

#endif
