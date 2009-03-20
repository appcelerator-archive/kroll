/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_REF_COUNTED_H_
#define _KR_REF_COUNTED_H_

#include "base.h"
#include "mutex.h"
#include "scoped_lock.h"

#ifdef DEBUG_REFCOUNT
	#define KR_CALL_STACK_DEFINE const char *filename = __FILE__, int linenumber = __LINE__, const char *func = KR_FUNC
	#define KR_CALL_STACK_INFO const char *filename, int linenumber, const char *func
	#define KR_CALL_STACK_DEBUG \
		static const char *__myfunc = KR_FUNC; \
		std::cout << "REF[" << \
		filename << "(" << func << "::" << linenumber << ") => " \
		__FILE__ << "(" << __myfunc << "::" << __LINE__ << ") => "<< (void*)this << "]" << \
		std::endl ;
#else
	#define KR_CALL_STACK_DEFINE
	#define KR_CALL_STACK_INFO
	#define KR_CALL_STACK_DEBUG
#endif


namespace kroll
{
	/**
	 * reference counted base class. this object should be created
	 * (reference count is initialized to 1 on construction) and never
	 * directly deleted.  to delete the object, call ReleaseReference()
	 * to release your reference.  once all references have been released
	 * (reference count = 0), the object will be automatically deleted.
	 * the methods on this object are thread-safe.
	 */
	class KROLL_API RefCounted
	{
	public:
		RefCounted(KR_CALL_STACK_DEFINE);
	protected:
		virtual ~RefCounted();
	public:

		/**
		 * TODO: Document me
		 */
		RefCounted* AddReference(KR_CALL_STACK_DEFINE);

		/**
		 * TODO: Document me
		 */
		void ReleaseReference(KR_CALL_STACK_DEFINE);

		/**
		 * TODO: Document me
		 */
		const int ReferenceCount();
	private:
#ifdef DEBUG_REFCOUNT
		const char *filename;
		int linenumber;
		const char *func;
#endif

		/**
		 * TODO: Document me
		 */
		int count;

		/**
		 * TODO: Document me
		 */
		Mutex mutex;
		DISALLOW_EVIL_CONSTRUCTORS(RefCounted);
	};
}

#endif

