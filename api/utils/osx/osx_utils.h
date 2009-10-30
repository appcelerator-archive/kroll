/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_OSX_UTILS_H_
#define _KR_OSX_UTILS_H_

#include <string>
#include <CoreFoundation/CoreFoundation.h>

// Most of the code was adapted from code in Chromium's src/base/...
namespace UTILS_NS
{
	KROLL_API std::string CFStringToUTF8(CFStringRef cfstring);
	KROLL_API CFStringRef UTF8ToCFString(const std::string& input);
	KROLL_API std::string CFErrorToString(CFErrorRef cferror);

	// scoped_cftyperef<> is patterned after scoped_ptr<>, but maintains ownership
	// of a CoreFoundation object: any object that can be represented as a
	// CFTypeRef.  Style deviations here are solely for compatibility with
	// scoped_ptr<>'s interface, with which everyone is already familiar.
	//
	// When scoped_cftyperef<> takes ownership of an object (in the constructor or
	// in reset()), it takes over the caller's existing ownership claim.  The 
	// caller must own the object it gives to scoped_cftyperef<>, and relinquishes
	// an ownership claim to that object.  scoped_cftyperef<> does not call
	// CFRetain().
	template<typename CFT>
	class scoped_cftyperef {
	public:
		typedef CFT element_type;
		explicit scoped_cftyperef(CFT object = NULL) :
			object_(object) { }

		~scoped_cftyperef()
		{
			if (object_)
				CFRelease(object_);
		}

		void reset(CFT object = NULL)
		{
			if (object_)
				CFRelease(object_);
			object_ = object;
		}

		bool operator==(CFT that) const
		{
			return object_ == that;
		}
	
		bool operator!=(CFT that) const
		{
			return object_ != that;
		}

		operator CFT() const
		{
			return object_;
		}

		CFT get() const
		{
			return object_;
		}

		void swap(scoped_cftyperef& that)
		{
			CFT temp = that.object_;
			that.object_ = object_;
			object_ = temp;
		}

		// scoped_cftyperef<>::release() is like scoped_ptr<>::release.  It is NOT
		// a wrapper for CFRelease().  To force a scoped_cftyperef<> object to call
		// CFRelease(), use scoped_cftyperef<>::reset().
		CFT release()
		{
			CFT temp = object_;
			object_ = NULL;
			return temp;
		}

	private:
		CFT object_;
	};

}
#endif
