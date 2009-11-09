/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_FUNCTION_PTR_METHOD_H_
#define _KR_FUNCTION_PTR_METHOD_H_

namespace kroll
{

	typedef KValueRef (*KFunctionPtrCallback) (const ValueList& args);
	class KROLL_API KFunctionPtrMethod : public KMethod
	{
		public:
		KFunctionPtrMethod(KFunctionPtrCallback);
		virtual ~KFunctionPtrMethod();

		/**
		 * @see KMethod::Call
		 */
		virtual KValueRef Call(const ValueList& args);

		/**
		 * @see KObject::Set
		 */
		virtual void Set(const char *name, KValueRef value);

		/**
		 * @see KObject::Get
		 */
		virtual KValueRef Get(const char *name);

		/**
		 * @see KObject::GetPropertyNames
		 */
		virtual SharedStringList GetPropertyNames();
		

		protected:
		KFunctionPtrCallback callback;
		KObjectRef object;

		private:
		DISALLOW_EVIL_CONSTRUCTORS(KFunctionPtrMethod);
	};
}

#endif
