/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_EVENT_H_
#define _KR_EVENT_H_

namespace kroll
{
	class KROLL_API Event : public AccessorBoundObject
	{
		public:
		Event(AutoPtr<KEventObject> target, std::string& eventName);
		void _GetTarget(const ValueList&, SharedValue result);
		void _GetType(const ValueList&, SharedValue result);
		void _GetTimestamp(const ValueList&, SharedValue result);
		void _StopPropagation(const ValueList&, SharedValue result);
		static void SetEventConstants(KObject* target);

		AutoPtr<KEventObject> target;
		std::string& eventName;
		Poco::Timestamp timestamp;
		bool stopped;
		static std::string ALL;
		static std::string FOCUSED;
		static std::string UNFOCUSED;
		static std::string OPEN;
		static std::string OPENED;
		static std::string CLOSE;
		static std::string CLOSED;
		static std::string HIDDEN;
		static std::string SHOWN;
		static std::string FULLSCREENED;
		static std::string UNFULLSCREENED;
		static std::string MAXIMIZED;
		static std::string MINIMIZED;
		static std::string RESIZED;
		static std::string MOVED;
		static std::string PAGE_INITIALIZED;
		static std::string PAGE_LOADED;
		static std::string CREATED;
		static std::string ACTIVATE;
		static std::string CLICKED;
		static std::string EXIT;
	};
}
#endif
