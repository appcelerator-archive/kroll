/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_EVENT_OBJECT_H_
#define _KR_EVENT_OBJECT_H_

namespace kroll
{
	class EventListener
	{
	public:
		EventListener(std::string& eventName, KMethodRef callback) :
			eventName(eventName),
			callback(callback),
			listenerId(++EventListener::currentId) {}
		void FireEventIfMatches(AutoPtr<Event>);
		bool Matches(std::string&, unsigned int, KMethodRef);
		std::string eventName;
		KMethodRef callback;
		unsigned int listenerId;

		static unsigned int currentId;
	};

	class KROLL_API KEventObject : public KAccessorObject
	{
	public:
		KEventObject(const char* name = "");
		~KEventObject();

		AutoPtr<Event> CreateEvent(const std::string& eventName);
		unsigned int AddEventListener(std::string& eventName, KMethodRef listener);
		unsigned int AddEventListenerForAllEvents(KMethodRef callback);
		virtual void RemoveEventListener(std::string& eventName, KMethodRef listener);
		virtual void RemoveEventListener(std::string& eventName, unsigned int id);
		virtual bool FireEvent(std::string& eventName);
		virtual bool FireEvent(AutoPtr<Event>);
		void _AddEventListener(const ValueList&, KValueRef result);
		void _RemoveEventListener(const ValueList&, KValueRef result);

	protected:
		static Poco::Mutex listenerMapMutex;
		static std::map<KEventObject*, std::vector<EventListener*>*> listenerMap;

	private:
		void RemoveEventListener(std::string& eventName, unsigned int id,
			KMethodRef callback);
	};

}

#endif
