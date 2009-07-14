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
		EventListener(std::string& eventName, SharedKMethod callback) :
			eventName(eventName),
			callback(callback),
			listenerId(++EventListener::currentId) {}
		void FireEventIfMatches(AutoPtr<Event>, bool synchronous = false);
		bool Matches(std::string&, unsigned int, SharedKMethod);
		std::string eventName;
		SharedKMethod callback;
		unsigned int listenerId;

		static unsigned int currentId;
	};

	class KROLL_API KEventObject : public AccessorBoundObject
	{
		public:
		KEventObject(const char* name = "");
		~KEventObject();

		unsigned int AddEventListener(std::string& eventName, SharedKMethod listener);
		unsigned int KEventObject::AddEventListenerForAllEvents(SharedKMethod callback);
		virtual void RemoveEventListener(std::string& eventName, SharedKMethod listener);
		virtual void RemoveEventListener(std::string& eventName, unsigned int id);
		virtual void FireEvent(std::string& eventName, bool synchronous = false);
		virtual void FireEvent(AutoPtr<Event>, bool synchronous = false);
		void _AddEventListener(const ValueList&, SharedValue result);
		void _RemoveEventListener(const ValueList&, SharedValue result);

		private:
		void RemoveEventListener(
			std::string& eventName, unsigned int id, SharedKMethod callback);

		private:
		std::vector<EventListener*>& GetListeners();
		static std::map<KEventObject*, std::vector<EventListener*>*> listenerMap;
	};

}

#endif
