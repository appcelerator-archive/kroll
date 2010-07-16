/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_OBJECT_H_
#define _KR_STATIC_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>

#include <Poco/Mutex.h>

namespace kroll
{
	/**
	 * Extending this class is the easiest way to get started with your own
	 * KObject implementation. In your sub-class' constructor, you can bind
	 * properties and methods, i.e:
	 * \code
	 * MyObject::MyObject() {
	 *   this->Set("x", Value::NewInt(100));
	 *   this->Set("description", Value::NewString("my object"));
	 *   this->SetMethod("add", &MyObject::Add);
	 * }
	 *
	 * void MyObject::Add(const ValueList& args, KValueRef result) {
	 *   result->SetInt(args[0]->ToInt() + args[1]->ToInt());
	 * }
	 * \endcode
	 *
	 * And a supported language would access your object ala:
	 * \code
	 * var myObject = //..
	 * alert(myObject.x); // 100
	 * alert(myObject.description); // "my object"
	 * alert(myObject.add(10, 15)); // 25
	 * \endcode
	 */
	class KROLL_API StaticBoundObject : public KObject
	{
	public:
		StaticBoundObject(const char* type = "StaticBoundObject");
		virtual ~StaticBoundObject();

		virtual bool HasProperty(const char* name);
		virtual KValueRef Get(const char* name);
		virtual SharedStringList GetPropertyNames();
		virtual void Set(const char* name, KValueRef value);
		virtual void Unset(const char* name);

		/**
		 * Set a property on this object to the given method. When an error
		 * occurs will throw an exception of type ValueException.
		 */
		template <typename T>
		void SetMethod(const char* name, void (T::*method)(const ValueList&, KValueRef))
		{
			this->Set(name, Value::NewMethod(new StaticBoundMethod(
				NewCallback<T, const ValueList&, KValueRef>(static_cast<T*>(this), method))));
		}


	protected:
		std::map<std::string, KValueRef> properties;
		Poco::Mutex mutex;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundObject);
	};

}

#endif
