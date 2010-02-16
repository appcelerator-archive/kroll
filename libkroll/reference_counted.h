/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_REFERENCE_COUNTED_H_
#define _KR_REFERENCE_COUNTED_H_

namespace kroll
{
	class KROLL_API ReferenceCounted
	{
		private:
		Poco::AtomicCounter count;

		public:
		ReferenceCounted() : count(1) { }
		virtual ~ReferenceCounted() { }

		virtual void duplicate()
		{
			++count;
		}

		virtual void release()
		{
			int value = --count;
			if (value <= 0) {
				delete this;
			}
		}

		virtual int referenceCount() const
		{
			return count.value();
		}
	};
}
#endif
