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
	/*
		Class: KObject
	*/
	class KROLL_API KObject
	{
	public:
		/*
			Constructor: KObject
		*/
		KObject() {}
		virtual ~KObject() {}

	public:
		/*
		 * Function: Set
		 *   Set a property on this object to the given value
		 *   Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/*
		 * Function: Get
		 *   Return the property with the given name or Value::Undefined
		 *   if the property is not found.
		 *   Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name) = 0;

		/*
		 * Function: GetPropertyNames
		 * Returns: a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/*
		 * Function: DisplayString
		 * Returns: a string representation of this object
		 */
		virtual SharedString DisplayString(int levels=3);

		/* Function: Set
		 *   Helpful overload to Set which takes a SharedString
		 */
		void Set(SharedString, SharedValue value);

		/* Function: Get
		 *   Helpful overload to Get which takes a SharedString
		 */
		SharedValue Get(SharedString);

		/*
		 * Function: SetNS
		 *   TODO: Document me
		 */
		void SetNS(const char *name, SharedValue value);

		/*
		 * Function: GetNS
		 *   TODO: Document me
		 */
		SharedValue GetNS(const char *name);

		/*
		 * Function: CallNS
		 * Invoke a fully qualified namespaced method passing arguments
		 */
		SharedValue CallNS(const char *name, SharedValue val1);
		SharedValue CallNS(const char *name, SharedValue val1, SharedValue val2);
		SharedValue CallNS(const char *name, SharedValue val1, SharedValue val2, SharedValue val3);
		SharedValue CallNS(const char *name, const ValueList& args);


		/**
		 * Function: GetString
		 *   Get a string property from this object.
		 *
		 * Params:
		 *   name - The name of the property to get
		 *   defaultValue - A value to return on failure
		 *
		 * Returns:
		 *   Value of given property name, or the default value if
		 *   if it does not exist or is not a string.
		 */
		std::string GetString(const char *name, std::string defaultValue);

		/**
		 * Function: GetBool
		 *   Get a boolean property from this object.
		 *
		 * Params:
		 *   name - The name of the property to get
		 *   defaultValue - A value to return on failure
		 *
		 * Returns:
		 *   Value of given property name, or the default value if
		 *   if it does not exist or is not a Bool.
		 */
		bool GetBool(const char *name, bool defaultValue);

		/**
		 * Function: GetStringList
		 *   Gets a list of strings for the given property for this object.  the list vector is gets the list of strings
		 *   appended to the end of the list
		 *
		 * Params:
		 *   name - the name of the property
		 *   list - the vector where the list of strings is appended to
		 *
		 * Returns:
		 *   void
		 */
		void GetStringList(const char *name, std::vector<std::string> &list);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KObject);
	};

}

#endif
