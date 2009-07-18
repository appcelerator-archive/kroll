/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_READ_EVENT_H_
#define _KR_READ_EVENT_H_

namespace kroll
{
	class KROLL_API ReadEvent : public Event
	{
		public:
		ReadEvent(AutoPtr<KEventObject> target, AutoBlob);
		void _GetData(const ValueList&, SharedValue result);

		protected:
		AutoBlob data;
	};
}
#endif
