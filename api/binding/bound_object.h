/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_OBJECT_H_
#define _KR_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	/*
		Class: BoundObject
	*/
	class KROLL_API BoundObject
	{
	public:
		/*
			Constructor: BoundObject
		*/
		BoundObject() { }
		virtual ~BoundObject() { }

		static SharedBoundObject CreateEmptyBoundObject();
	public:
		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference,
		  if they increase the reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/*
			Function: Get

		  Return an object's property. The returned value is automatically
		  reference counted and must be released if the callee does not hold
		  a reference (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Get(const char *name) = 0;

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;


		/* Function: Set

			Helpful overload to Set which takes a SharedString
		 */
		void Set(SharedString, SharedValue value);

		/* Function: Get

			Helpful overload to Get which takes a SharedString
		 */
		SharedValue Get(SharedString);

		/*
			Function: SetNS

			TODO: Document me
		*/
		void SetNS(const char *name, SharedValue value);

		/*
			Function: GetNS

			TODO: Document me
		*/
		SharedValue GetNS(const char *name);

		/*
			Function: CallNS

			TODO: Document me
		 */
		SharedValue CallNS(const char *name, SharedValue val1);
		SharedValue CallNS(const char *name, SharedValue val1, SharedValue val2);
		SharedValue CallNS(const char *name, SharedValue val1, SharedValue val2, SharedValue val3);
		SharedValue CallNS(const char *name, const ValueList& args);

		/**
		 * Function: DisplayString
		 *
		 * Return a string representation of this object
		 */
		SharedString DisplayString(int levels=3);

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
		DISALLOW_EVIL_CONSTRUCTORS(BoundObject);
	};

}

#endif
