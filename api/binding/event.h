/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_EVENT_H_
#define _KR_EVENT_H_

namespace kroll
{
	class KROLL_API Event : public KAccessorObject
	{
	public:
		Event(AutoPtr<KEventObject> target, const std::string& eventName);
		void _GetTarget(const ValueList&, KValueRef result);
		void _GetType(const ValueList&, KValueRef result);
		void _GetTimestamp(const ValueList&, KValueRef result);
		void _StopPropagation(const ValueList&, KValueRef result);
		void _PreventDefault(const ValueList&, KValueRef result);
		static void SetEventConstants(KObject* target);

		AutoPtr<KEventObject> target;
		std::string eventName;
		Poco::Timestamp timestamp;
		bool stopped;
		bool preventedDefault;
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
		static std::string DOUBLE_CLICKED;
		static std::string EXIT;
		static std::string APP_EXIT;
		static std::string READ;
		static std::string HTTP_DONE;
		static std::string HTTP_STATE_CHANGED;
		static std::string HTTP_TIMEOUT;
		static std::string HTTP_REDIRECT;
		static std::string HTTP_ABORT;
		static std::string HTTP_DATA_SENT;
		static std::string HTTP_DATA_RECEIVED;
		static std::string OPEN_REQUEST;
	};
}
#endif
