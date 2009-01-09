/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_VALUE_H_
#define _KR_VALUE_H_

#include "../ref_counted.h"
#include "binding.h"


namespace kroll
{
	/**
	 * This typdef is only used for argument lists. For
	 * a list implementation to be used as a value in the
	 * binding layer, take a look at BoundList and
	 * StaticBoundList.
	 */
	typedef std::vector<Value*> ValueList;

	/**
	 * Value is a container object which internally contains
	 * a value which can be boxed/unboxed based on the type.
	 */
	class KROLL_API Value : public RefCounted
	{
	public:
	
		/**
		 * enumeration of value types
		 */
		enum Type {
			INT = 1,
			DOUBLE = 2,
			BOOL = 3,
			STRING = 4,
			LIST = 5,
			OBJECT = 6,
			METHOD = 7,
			NULLV = 0,
			UNDEFINED = -1
		};

		/**
		 * construct an UNDEFINED type
		 */
		Value();

		/**
		 * construct an INT type
		 */
		Value(int value);

		/**
		 * construct a DOUBLE type
		 */
		Value(double value);

		/**
		 * construct a BOOL type
		 */
		Value(bool value);

		/**
		 * construct a STRING type
		 */
		Value(const char* value);

		/**
		 * construct an STRING type
		 */
		Value(std::string& value);

		/**
		 * construct a LIST type
		 */
		Value(BoundList* value);

		/**
		 * construct an OBJECT type
		 */
		Value(BoundObject* value);

		/**
		 * construct a METHOD type
		 */
		Value(BoundMethod* value);

		/**
		 * construct a copy constructor for an existing Value
		 */
		Value(const Value& value);

	protected:
		/**
		 * destructor
		 */
		virtual ~Value();
		
	public:
	
		/**
		 * return a system-defined shared UNDEFINED type. the
		 * return pointer is not borrowed and must referenced
		 * counted if used or returned from a method call.
		 */
		static Value* Undefined();
	
		/**
		 * return a system-defined shared NULL type. the
		 * return pointer is not borrowed and must referenced
		 * counted if used or returned from a method call.
		 */
		static Value* Null();

		bool operator== (Value);

		/**
		 * return true if the internal value is an INT
		 */
		bool IsInt() const;
	
		/**
		 * return true if the internal value is a DOUBLE
		 */
		bool IsDouble() const;

		/**
		 * return true if the internal value is a BOOL
		 */
		bool IsBool() const;

		/**
		 * return true if the internal value is a STRING
		 */
		bool IsString() const;

		/**
		 * return true if the internal value is a LIST
		 */
		bool IsList() const;

		/**
		 * return true if the internal value is a OBJECT
		 */
		bool IsObject() const;

		/**
		 * return true if the internal value is a METHOD
		 */
		bool IsMethod() const;

		/**
		 * return true if the internal value is a NULL
		 */
		bool IsNull() const;

		/**
		 * return true if the internal value is an UNDEFINED
		 */
		bool IsUndefined() const;

		/**
		 * return the value as an int
		 */
		int ToInt() const;
	
		/**
		 * return the value as a double
		 */
		double ToDouble() const;
	
		/**
		 * return the value as a bool
		 */
		bool ToBool() const;
	
		/**
		 * return the value as a std::string
		 */
		std::string ToString() const;
	
		/**
		 * return the value as a BoundList
		 */
		BoundList* ToList() const;
	
		/**
		 * return the value as a BoundObject*
		 */
		BoundObject* ToObject() const;
	
		/**
		 * return the value as a BoundMethod*
		 */
		BoundMethod* ToMethod() const;

		/**
		 * return the type name of the internal value
		 */
		const char* ToTypeString();

		/**
		 * changes the internal value of this instance.
		 * copy the internal value of other into this object.
		 * this instance will make a reference of the internal
		 * value of other but not other itself
		 */
		void Set(Value* other);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(int value);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(double value);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(bool value);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(std::string value);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(BoundList* value);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(BoundObject* value);

		/**
		 * change the internal value of this instance to value
		 */
		void Set(BoundMethod* value);

		/**
		 * change the internal value of this instance to NULL
		 */
		void SetNull();

		/**
		 * change the internal value of this instance to UNDEFINED
		 */
		void SetUndefined();
	
	private:
		Type type;
		double numberValue;
		bool boolValue;
		std::string stringValue;
		BoundObject *objectValue;
		void defaults();
		void init();
		
	};
}

#endif

