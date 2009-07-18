/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_REFERNCE_COUNTED_H_
#define _KR_REFERNCE_COUNTED_H_

namespace kroll
{
	class KROLL_API ReferenceCounted
	{
		private:
		Poco::AtomicCounter referenceCount;

		public:
		ReferenceCounted() : referenceCount(1) { }
		virtual ~ReferenceCounted() { }

		virtual void duplicate()
		{
			referenceCount++;
		}

		virtual void release()
		{
			referenceCount--;
			if (referenceCount.value() <= 0) {
				delete this;
			}
		}
	};
}
#endif
