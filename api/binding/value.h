/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_VALUE_H_
#define _KR_VALUE_H_

namespace kroll
{
	/*
		Class: Value
	  Value is a container object which internally contains
	  a value which can be boxed/unboxed based on the type.
	 */
	class KROLL_API Value
	{
	public:

		/*
		 * Enum: Type
		 * INT - Integer
		 * DOUBLE - Double
		 * BOOL - Boolean
		 * STRING - String
		 * LIST - <ValueList>
		 * OBJECT - <KObject>
		 * METHOD - <KMethod>
		 * VOIDP - void * (used for raw types internally)
		 * NULLV - Null
		 * UNDEFINED - Undefined
		 */
		enum Type {
			INT = 1,
			DOUBLE = 2,
			BOOL = 3,
			STRING = 4,
			LIST = 5,
			OBJECT = 6,
			METHOD = 7,
			VOIDPTR = 8,
			NULLV = 0,
			UNDEFINED = -1
		};

		/*
		 * An undefined singleton
		 */
		static SharedValue Undefined;

		/*
		 * A Null singleton
		 */
		static SharedValue Null;

		/*
		 * Function: NewUndefined
		 * Create a new Value set to Undefined
		 */
		static SharedValue NewUndefined();

		/*
		 * Function: NewNull
		 * Create a new Value set to Null
		 */
		static SharedValue NewNull();

		/*
		 * Constructor: Value
		 * construct an <INT> type
		 */
		static SharedValue NewInt(int value);

		/**
			Constructor: Value
		  construct a <DOUBLE> type
		 */
		static SharedValue NewDouble(double value);

		/*
			Constructor: Value
		  construct a <BOOL> type
		 */
		static SharedValue NewBool(bool value);

		/*
			Constructor: Value
		  construct a <STRING> type
		 */
		static SharedValue NewString(const char* value);

		/*
			Constructor: Value
		  construct an <STRING> type
		 */
		static SharedValue NewString(std::string& value);

		/*
			Constructor: Value
		  construct a <LIST> type
		 */
		static SharedValue NewList(SharedKList value);

		/*
			Constructor: Value
		  construct an <OBJECT> type
		 */
		static SharedValue NewObject(SharedKObject value);

		/*
			Constructor: Value
		  construct a <METHOD> type
		 */
		static SharedValue NewMethod(SharedKMethod value);

		/*
			Constructor: Value
			construct a <VOIDPTR> type
		 */
		static SharedValue NewVoidPtr(void *value);

		/**
		 * destructor
		 */
		virtual ~Value();

	public:
		/*
		 * Function: operator==
		 */
		bool operator== (Value);

		/*
		 * Function: IsInt
		 *   Return true if the internal value is an INT
		 */
		bool IsInt() const;

		/*
		 * Function: IsDouble
		 *   Return true if the internal value is a DOUBLE
		 */
		bool IsDouble() const;

		/*
		 * Function: IsNumber
		 *   Return true if the internal value is an INT or a DOUBLE
		 */
		bool IsNumber() const;

		/*
		 * Function: IsBool
		 *  Return true if the internal value is a BOOL
		 */
		bool IsBool() const;

		/*
		 * Function: IsString
		 *   Return true if the internal value is a STRING
		 */
		bool IsString() const;

		/*
		 * Function: IsList
		 *   Return true if the internal value is a LIST
		 */
		bool IsList() const;

		/*
		 * Function: IsObject
		 *   Return true if the internal value is a OBJECT
		 */
		bool IsObject() const;

		/*
		 * Function: IsMethod
		 *   Return true if the internal value is a METHOD
		 */
		bool IsMethod() const;

		/*
		 * Function: IsVoidPtr
		 *   Return true if the internal value is a VOIDPTR
		 */
		bool IsVoidPtr() const;

		/*
		 * Function: IsNull
		 *   Return true if the internal value is a NULL
		 */
		bool IsNull() const;

		/*
		 * Function: IsUndefined
		 *   Return true if the internal value Undefined
		 */
		bool IsUndefined() const;

		/*
		 * Function: ToInt
		 *   Return the value as an int
		 */
		int ToInt() const;

		/*
		 * Function: ToDouble
		 *   Return the value as a double
		 */
		double ToDouble() const;

		/*
		 * Function: ToNumber
		 *  Return the double value of a value when it is an int
		 *  or a dobule.
		 */
		double ToNumber() const;

		/*
		 * Function: ToBool
		 *   Return the value as a bool
		 */
		bool ToBool() const;

		/*
		 * Function: ToString
		 *   Return the value as a const char*
		 */
		const char* ToString() const;

		/*
		 * Function: ToList
		 *   Return the value as a SharedKList
		 */
		SharedKList ToList() const;

		/*
		 * Function: ToObject
		 *   Return the value as a SharedKObject
		 */
		SharedKObject ToObject() const;

		/*
		 * Function: ToMethod
		 *   Return the value as a SharedKMethod
		 */
		SharedKMethod ToMethod() const;

		/*
		 * Function: ToVoidPtr
		 *   Return the value as a void **
		 */
		void* ToVoidPtr() const;

		/*
		 * Function: ToTypeString
		 *   Create a string representation of this Value's type
		 */
		const char* ToTypeString();

		/*
		 * Function: DisplayString
		 *   Create a string representation for this Value
		*/
		SharedString DisplayString(int levels=3);

		/*
		 * Function: SetValue
		 *   Change the internal value of this Value to match th
		 *   one in the given Value, other.
		 */
		void SetValue(Value *other);

		/*
		 * Function: SetValue
		 *   Change the internal value of this Value to match th
		 *   one in the given Value, other.
		 */
		void SetValue(SharedValue other);

		/*
		 * Function: SetInt
		 *   Change the internal value of this Value to the given int
		 */
		void SetInt(int value);

		/*
		 * Function: SetDouble
		 *   Change the internal value of this Value to the given double
		 */
		void SetDouble(double value);

		/*
		 * Function: Set
		 *   Change the internal value of this Value to the given bool
		 */
		void SetBool(bool value);

		/*
		 * Function: Set
		 *   Change the internal value of this Value to the given string
		 */
		void SetString(const char* value);

		/*
		 * Function: Set
		 *   Change the internal value of this Value to the given string
		 */
		void SetString(std::string& value);

		/*
		 * Function: Set
		 *   Change the internal value of this Value to the given SharedKList
		 */
		void SetList(SharedKList value);

		/*
		 * Function: Set
		 *   Change the internal value of this Value to the given SharedKObject
		 */
		void SetObject(SharedKObject value);

		/**
		 * Function: Set
		 *   Change the internal value of this Value to the given SharedKMethod
		 */
		void SetMethod(SharedKMethod value);

		/*
		 * Function: Set
		 *   Change the internal value of this Value to the given void pointer
		 */
		void SetVoidPtr(void *value);

		/*
		 * Function: SetNull
		 *   Change the internal value of this Value to Null
		 */
		void SetNull();

		/*
		 * Function: SetUndefined
		 *   Change the internal value of this Value to Undefined
		 */
		void SetUndefined();

	private:
		Type type;
		double numberValue;
		bool boolValue;
		char* stringValue;
		SharedKObject objectValue;
		void *voidPtrValue;

		void reset();

		/*
		 * Constructor: Value
		 *   the Value constructor is private to force the user of factory
		 *   methods to create Values. The default state of a value is Undefined.
		 */
		Value();

		/*
		 * Constructor: Value
		 *  Copy constructor
		 */
		Value(SharedValue value);

		/*
		 * Constructor: Value
		 *   Copy constructor
		 */
		Value(const Value& value);
	};

	class KROLL_API ValueReleasePolicy : public Poco::ReleasePolicy<Value> {
	public:
		static void release(Value* pObj)
		{
			delete pObj;
			pObj = NULL;
		}
	};

}

#endif

