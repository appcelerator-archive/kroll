/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_LIST_H_
#define _KR_ACCESSOR_BOUND_LIST_H_

namespace kroll
{
	/**
	 * The AccesorBoundList allows you to expose a list with properties that automatically expose getters and setters
	 * @see AccessorBoundObject
	 */
	class KROLL_API AccessorBoundList : public StaticBoundList
	{
	public:

		AccessorBoundList();

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
		DISALLOW_EVIL_CONSTRUCTORS(AccessorBoundList);
		static std::string Capitalize(const char *word);
	};
}

#endif
