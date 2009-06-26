/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_CORE_TYPES_H_
#define _KR_CORE_TYPES_H_

namespace kroll
{
	/**
	 * An object which exposes core types to the API for testing and recreation
	 */
	class CoreTypes : public StaticBoundObject
	{
	public:
		CoreTypes();
		~CoreTypes();

	private:
		void CreateKObject(const ValueList& args, SharedValue result);
		void CreateKMethod(const ValueList& args, SharedValue result);
		void CreateKList(const ValueList& args, SharedValue result);
		void CreateBlob(const ValueList& args, SharedValue result);
	};

	/**
	 * An wrapper for a KObject which encapsulates another one for testing
	 */
	class KObjectWrapper : public KObject
	{
	public:
		KObjectWrapper(SharedKObject object);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);

	private:
		SharedKObject object;
	};

	/**
	 * An wrapper for a KMethod which encapsulates another one for testing
	 */
	class KMethodWrapper : public KMethod
	{
	public:
		KMethodWrapper(SharedKMethod method);
		SharedValue Call(const ValueList& args);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);

	private:
		SharedKMethod method;
	};

	/**
	 * An wrapper for a KList which encapsulates another one for testing
	 */
	class KListWrapper : public KList
	{
	public:
		KListWrapper(SharedKList list);
		void Append(SharedValue value);
		unsigned int Size();
		SharedValue At(unsigned int index);
		void SetAt(unsigned int index, SharedValue value);
		bool Remove(unsigned int index);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels=3);

	private:
		SharedKList list;
	};
}

#endif
