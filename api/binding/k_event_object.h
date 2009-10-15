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
		void FireEventIfMatches(AutoPtr<Event>);
		bool Matches(std::string&, unsigned int, SharedKMethod);
		std::string eventName;
		SharedKMethod callback;
		unsigned int listenerId;

		static unsigned int currentId;
	};

	class KROLL_API KEventObject : public KAccessorObject
	{
		public:
		KEventObject(const char* name = "");
		KEventObject(bool root, const char* name = "");
		~KEventObject();

		AutoPtr<Event> CreateEvent(const std::string& eventName);
		unsigned int AddEventListener(std::string& eventName, SharedKMethod listener);
		unsigned int AddEventListenerForAllEvents(SharedKMethod callback);
		virtual void RemoveEventListener(std::string& eventName, SharedKMethod listener);
		virtual void RemoveEventListener(std::string& eventName, unsigned int id);
		virtual bool FireEvent(std::string& eventName);
		virtual bool FireEvent(AutoPtr<Event>);
		void _AddEventListener(const ValueList&, SharedValue result);
		void _RemoveEventListener(const ValueList&, SharedValue result);
		static bool FireRootEvent(std::string& eventName);
		static bool FireRootEvent(AutoPtr<Event>);
		static AutoPtr<KEventObject> root;

		protected:
		bool isRoot;
		static Poco::Mutex listenerMapMutex;
		static std::map<KEventObject*, std::vector<EventListener*>*> listenerMap;

		private:
		void RemoveEventListener(
			std::string& eventName, unsigned int id, SharedKMethod callback);
	};

}

#endif
