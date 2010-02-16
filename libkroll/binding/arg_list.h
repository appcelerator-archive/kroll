/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_ARG_LISTT_H_
#define _K_ARG_LISTT_H_

#include <vector>
#include <string>
#include <map>
#include "callback.h"

namespace kroll
{
	/**
	 * An argument list
	 *
	 * This class is only used for argument lists. For a list implementation to be
	 *  used as a value in the binding layer, take a look at KList and StaticBoundList.
	 */
	class KROLL_API ArgList
	{
		public:
		ArgList();
		ArgList(KValueRef);
		ArgList(KValueRef, KValueRef);
		ArgList(KValueRef, KValueRef, KValueRef);
		ArgList(KValueRef, KValueRef, KValueRef, KValueRef);
		ArgList(const ArgList&);
		~ArgList() {};

		bool Verify(std::string& argSpec) const;
		void VerifyException(const char* name, std::string argSpec) const;

		public:
		void push_back(KValueRef value);
		size_t size() const;
		const KValueRef& at(size_t) const;
		const KValueRef& operator[](size_t) const;

		KValueRef GetValue(size_t index, KValueRef defaultValue=Value::Undefined) const;
		int GetInt(size_t index, int defaultValue=0) const;
		double GetDouble(size_t index, double defaultValue=0.0) const;
		double GetNumber(size_t index, double defaultValue=0.0) const;
		bool GetBool(size_t index, bool defaultValue=false) const;
		std::string GetString(size_t index, std::string defaultValue="") const;
		KObjectRef GetObject(size_t index, KObjectRef defaultValue=NULL) const;
		KMethodRef GetMethod(size_t index, KMethodRef defaultValue=NULL) const;
		KListRef GetList(size_t index, KListRef defaultValue=NULL) const;

		private:
		SharedPtr<std::vector<KValueRef> > args;

		static inline bool VerifyArg(KValueRef arg, char t);
		static std::string GenerateSignature(const char* name, std::string& argSpec);
	};

}

#endif
