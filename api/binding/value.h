/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_VALUE_H_
#define _KR_VALUE_H_

/**
 * The toplevel kroll namespace.
 */
namespace kroll
{
	/**
	 * A container for various types.
	 * Value instances contain a primitive or object value which can be boxed/unboxed based on the type.
	 */
	class KROLL_API Value
	{
	public:

		/**
		 * Type enum.
		 * This enum represents the underlying value type.
		 */
		enum Type {
			INT = 1, /**< int */
			DOUBLE = 2, /**< double */
			BOOL = 3, /**< boolean */
			STRING = 4, /**< const char* */
			LIST = 5, /**< SharedKList */
			OBJECT = 6, /**< SharedKObject */
			METHOD = 7, /**< SharedKMethod */
			VOIDPTR = 8, /**< void* */
			NULLV = 0, /**< NULL */
			UNDEFINED = -1 /**< undefined */
		};

		/**
		 * A static, reusable instance who's value is Value::Type::UNDEFINED.
		 */
		static SharedValue Undefined;

		/**
		 * A static, reusable instance who's value is Value::Type::NULL
		 */
		static SharedValue Null;

		static SharedValue NewUndefined();
		static SharedValue NewNull();

		/**
		 * Construct a new \link #Value::Type::INT integer\endlink value.
		 * @param value The integer value
		 */
		static SharedValue NewInt(int value);

		/**
		 * Construct a new \link #Value::Type::DOUBLE double\endlink value.
		 * @param value The double value
		 */
		static SharedValue NewDouble(double value);

		/**
		 * Construct a new \link #Value::Type::BOOL boolean\endlink value.
		 * @param value The boolean value
		 */
		static SharedValue NewBool(bool value);

		/**
		 * Construct a new \link #Value::Type::STRING string\endlink value.
		 * @param value The string value
		 */
		static SharedValue NewString(const char* value);

		/**
		 * Construct a new \link #Value::Type::STRING string\endlink value.
		 * @param value The string value
		 */
		static SharedValue NewString(std::string& value);

		/**
		 * Construct a new \link #Value::Type::LIST list\endlink value.
		 * @param value The list value
		 */
		static SharedValue NewList(SharedKList value);

		/**
		 * Construct a new \link #Value::Type::OBJECT object\endlink value.
		 * @param value The object value
		 */
		static SharedValue NewObject(SharedKObject value);

		/**
		 * Construct a new \link #Value::Type::METHOD method\endlink value.
		 * @param value The method value
		 */
		static SharedValue NewMethod(SharedKMethod value);

		/**
		 * Construct a new \link #Value::Type::VOIDPTR void*\endlink value.
		 * @param value The void* value
		 */
		static SharedValue NewVoidPtr(void *value);

		virtual ~Value();

	public:
		/**
		 * Test underlying value's equality to another Value
		 */
		bool operator== (Value);

		/**
		 * @return true if the internal value is an \link #Value::Type::INT integer\endlink
		 */
		bool IsInt() const;

		/**
		 * @return true if the internal value is a \link #Value::Type::DOUBLE double\endlink
		 */
		bool IsDouble() const;

		/**
		 * @return true if the internal value is an \link #Value::Type::INT integer\endlink or \link #Value::Type::DOUBE double\endlink
		 */
		bool IsNumber() const;

		/**
		 * @return true if the internal value is a \link #Value::Type::BOOL boolean\endlink
		 */
		bool IsBool() const;

		/**
		 * @return true if the internal value is a \link #Value::Type::STRING string\endlink
		 */
		bool IsString() const;

		/**
		 * @return true if the internal value is a \link #Value::Type::LIST list\endlink
		 */
		bool IsList() const;

		/**
		 * @return true if the internal value is an \link #Value::Type::OBJECT object\endlink
		 */
		bool IsObject() const;

		/**
		 * @return true if the internal value is a \link #Value::Type::METHOD method\endlink
		 */
		bool IsMethod() const;

		/**
		 * @return true if the internal value is a \link #Value::Type::VOIDPTR void*\endlink
		 */
		bool IsVoidPtr() const;

		/**
		 * @return true if the internal value is \link #Value::Type::NULL NULL\endlink
		 */
		bool IsNull() const;

		/**
		 * @return true if the internal value is \link #Value::Type::UNDEFINED undefined\endlink
		 */
		bool IsUndefined() const;

		/**
		 * @return the value as an \link #Value::Type::INT integer\endlink
		 */
		int ToInt() const;

		/**
		 * @return the value as a \link #Value::Type::DOUBLE double\endlink
		 */
		double ToDouble() const;

		/**
		 * @return the double value when this value is an \link #Value::Type::INT integer\endlink or a \link #Value::Type::DOUBLE double\endlink.
		 */
		double ToNumber() const;

		/**
		 * @return the value as a \link #Value::Type::BOOL boolean\endlink
		 */
		bool ToBool() const;

		/**
		 * @return the value as a \link #Value::Type::STRING string (const char *)\endlink
		 */
		const char* ToString() const;

		/**
		 * @return the value as a \link #Value::Type::LIST SharedKList\endlink
		 */
		SharedKList ToList() const;

		/**
		 * @return the value as a \link #Value::Type::OBJECT SharedKObject\endlink
		 */
		SharedKObject ToObject() const;

		/**
		 * @return the value as a \link #Value::Type::METHOD SharedKMethod\endlink
		 */
		SharedKMethod ToMethod() const;

		/**
		 * @return the value as a \link #Value::Type::VOIDPTR void*\endlink
		 */
		void* ToVoidPtr() const;

		/**
		 * @return a string representation of this Value's type
		 */
		const char* ToTypeString();

		/**
		 * @param levels the number of nested objects to include in this representation (default: 3)
		 * @return a string representation for this Value
		*/
		SharedString DisplayString(int levels=3);

		/**
		 * Change the internal value of this Value from another Value object.
		 * @param other another Value
		 */
		void SetValue(Value *other);

		/**
		 * Change the internal value of this Value from another Value object.
		 * @param other another Value
		 */
		void SetValue(SharedValue other);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::INT integer\endlink
		 * @param value the integer value
		 */
		void SetInt(int value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::DOUBLE double\endlink
		 * @param value the double value
		 */
		void SetDouble(double value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::BOOL boolean\endlink
		 * @param value the boolean value
		 */
		void SetBool(bool value);

		/**
		 * Change the internal value of this Value to a \link #Value::Type::STRING string\endlink
		 * @param value the string value value
		 */
		void SetString(const char* value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::STRING string\endlink
		 * @param value the string value
		 */
		void SetString(std::string& value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::LIST list\endlink
		 * @param value the list value
		 */
		void SetList(SharedKList value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::OBJECT object\endlink
		 * @param value the object value
		 */
		void SetObject(SharedKObject value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::METHOD method\endlink
		 * @param value the method value
		 */
		void SetMethod(SharedKMethod value);

		/**
		 * Change the internal value of this Value to an \link #Value::Type::VOIDPTR void*\endlink
		 * @param value the void* value
		 */
		void SetVoidPtr(void *value);

		/**
		 * Change the internal value of this Value to \link #Value::Type::NULL NULL\endlink
		 */
		void SetNull();

		/**
		 * Change the internal value of this Value to \link #Value::Type::Undefined undefined\endlink
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

		Value();
		Value(SharedValue value);
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

