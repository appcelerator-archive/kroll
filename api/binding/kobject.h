/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_OBJECT_H_
#define _K_OBJECT_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	/**
	 * An abstract object.
	 *
	 * In general, KObject implementations follow two patterns:
	 * - Dynamic implementation (subclassing KObject directly)
	 *   This allows the implementation to do custom logic for specific
	 *   properties, but is also more verbose
	 * - Static implementation (subclassing StaticBoundObject)
	 *   This implementation uses an internal map to bind property names
	 *   to \link Value Values\endlink (objects, methods, etc).
	 */
	class KROLL_API KObject
	{
	public:
		KObject() {}
		virtual ~KObject() {}

	public:
		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference.
		 * When an error occurs will throw an exception of type ValueException.
		 * @param name The property name
		 * @param value The new property value
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/**
		 * @param name The property name
		 * @return the value of the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name) = 0;

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/**
		 * @param levels The number of levels of children to display in this string (default: 3)
		 * @return a string representation of this object
		 */
		virtual SharedString DisplayString(int levels=3);

		/**
		 * @param name The property name
		 * @param value The new property value
		 * Helpful overload to Set which takes a SharedString
		 */
		void Set(SharedString name, SharedValue value);

		/**
		 * @see KObject::Get(const char*)
		 * @param name The property name
		 * Helpful overload to Get which takes a SharedString
		 */
		SharedValue Get(SharedString name);

		/**
		 * Set the value of a child of this object using a simple object notation
		 * For example:
		 * \code
		 * this->SetNS("object.property.subproperty", value);
		 * // instead of
		 * this->Get("object")->ToObject()->Get("property")->ToObject()->Get("subproperty")->ToObject()->SetValue(value);
		 * \endcode
		 *
		 * This function does nothing if the object or it's parents are undefined
		 */
		void SetNS(const char *name, SharedValue value);

		/**
		 * Get the value of a child of this object using a simple object notation
		 * For example:
		 * \code
		 * SharedValue value = this->GetNS("object.property.subproperty");
		 * // instead of
		 * SharedValue value = this->Get("object")->ToObject()->Get("property")->ToObject()->Get("subproperty");
		 * \endcode
		 *
		 * @return The value of the child object, or Value::Undefined if the object, or it's parents are undefined
		 */
		SharedValue GetNS(const char *name);

		/**
		 * Call a child method on this object using simple object notation
		 * For example:
		 * \code
		 * this->CallNS("object.property.method", value1, value2);
		 * // instead of
		 * ValueList args;
		 * args.push_back(value1);
		 * args.push_back(value2);
		 * this->Get("object")->ToObject()->Get("property")->ToObject()->Get("method")->ToMethod()->Call(args);
		 * \endcode
		 *
		 * @return The return value of the function, or Value::Undefined if the object or method in the string is undefined
		 */
		SharedValue CallNS(const char *name, SharedValue val1);

		/**
		 * @see KObject::CallNS(const char *name, SharedValue val1);
		 */
		SharedValue CallNS(const char *name, SharedValue val1, SharedValue val2);

		/**
		 * @see KObject::CallNS(const char *name, SharedValue val1);
		 */
		SharedValue CallNS(const char *name, SharedValue val1, SharedValue val2, SharedValue val3);

		/**
		 * @see KObject::CallNS(const char *name, SharedValue val1);
		 */
		SharedValue CallNS(const char *name, const ValueList& args);


		/**
		 * Get a string property from this object.
		 *
		 * @param name The name of the property to get
		 * @param defaultValue A value to return on failure
		 *
		 * @return Value of given property name, or the default value if
		 * if it does not exist or is not a string.
		 */
		std::string GetString(const char *name, std::string defaultValue);

		/**
		 * Get a boolean property from this object.
		 *
		 * @param name The name of the property to get
		 * @param defaultValue A value to return on failure
		 *
		 * @return Value of given property name, or the default value if
		 * if it does not exist or is not a Bool.
		 */
		bool GetBool(const char *name, bool defaultValue);

		/**
		 * Gets a list of strings for the given property of this object. The list
		 * is appended to the end of the passed-in list.
		 *
		 * @param name the name of the property
		 * @param list the vector where the list of strings is appended to
		 */
		void GetStringList(const char *name, std::vector<std::string> &list);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KObject);
	};

}

#endif
