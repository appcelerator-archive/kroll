/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{
	KEventObject::KEventObject(const char *type) :
		KAccessorObject(type)
	{
		this->SetMethod("on", &KEventObject::_AddEventListener);
		this->SetMethod("addEventListener", &KEventObject::_AddEventListener);
		this->SetMethod("removeEventListener", &KEventObject::_RemoveEventListener);

		Event::SetEventConstants(this);
	}

	KEventObject::~KEventObject()
	{
		this->RemoveAllEventListeners();
	}

	AutoPtr<Event> KEventObject::CreateEvent(const std::string& eventName)
	{
		return new Event(AutoPtr<KEventObject>(this, true), eventName);
	}

	void KEventObject::AddEventListener(std::string& event, KMethodRef callback)
	{
		Poco::FastMutex::ScopedLock lock(this->listenersMutex);
		listeners.push_back(new EventListener(event, callback));
	}

	void KEventObject::RemoveEventListener(std::string& event, KMethodRef callback)
	{
		Poco::FastMutex::ScopedLock lock(this->listenersMutex);

		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			EventListener* listener = *i;
			if (listener->Handles(event) && listener->Callback()->Equals(callback))
			{
				this->listeners.erase(i);
				delete listener;
				break;
			}
			i++;
		}
	}

	void KEventObject::RemoveAllEventListeners()
	{
		Poco::FastMutex::ScopedLock lock(this->listenersMutex);

		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			delete *i++;
		}

		this->listeners.clear();
	}

	void KEventObject::FireEvent(std::string& event, const ValueList& args)
	{
		// Make a copy of the listeners map here, because firing the event might
		// take a while and we don't want to block other threads that just need
		// too add event listeners.
		EventListenerList listenersCopy;
		{
			Poco::FastMutex::ScopedLock lock(this->listenersMutex);
			listenersCopy = listeners;
		}

		KObjectRef thisObject(this, true);
		EventListenerList::iterator li = listenersCopy.begin();
		while (li != listenersCopy.end())
		{
			EventListener* listener = *li++;
			if (listener->Handles(event))
			{
				try
				{
					if (!listener->Dispatch(thisObject, args, true))
					{
						// Stop event dispatch if callback tells us
						break;
					}
				}
				catch (ValueException& e)
				{
					this->ReportDispatchError(e.ToString());
					break;
				}
			}
		}	
	}

	bool KEventObject::FireEvent(std::string& eventName, bool synchronous)
	{
		AutoPtr<Event> event(this->CreateEvent(eventName));
		return this->FireEvent(event);
	}

	bool KEventObject::FireEvent(AutoPtr<Event> event, bool synchronous)
	{
		// Make a copy of the listeners map here, because firing the event might
		// take a while and we don't want to block other threads that just need
		// too add event listeners.
		EventListenerList listenersCopy;
		{
			Poco::FastMutex::ScopedLock lock(listenersMutex);
			listenersCopy = listeners;
		}

		KObjectRef thisObject(this, true);
		EventListenerList::iterator li = listenersCopy.begin();
		while (li != listenersCopy.end())
		{
			EventListener* listener = *li++;
			if (listener->Handles(event->eventName))
			{
				ValueList args(Value::NewObject(event));
				bool result;

				try
				{
					result = listener->Dispatch(thisObject, args, synchronous);
				}
				catch (ValueException& e)
				{
					this->ReportDispatchError(e.ToString());
				}

				if (synchronous && (event->stopped || !result))
					return !event->preventedDefault;
			}
		}

		if (this != GlobalObject::GetInstance().get())
			GlobalObject::GetInstance()->FireEvent(event, synchronous);

		return !synchronous || !event->preventedDefault;
	}

	void KEventObject::_AddEventListener(const ValueList& args, KValueRef result)
	{
		std::string event;
		KMethodRef callback;

		if (args.size() > 1 && args.at(0)->IsString() && args.at(1)->IsMethod())
		{
			event = args.GetString(0);
			callback = args.GetMethod(1);
		}
		else if (args.size() > 0 && args.at(0)->IsMethod())
		{
			event = Event::ALL;
			callback = args.GetMethod(0);
		}
		else
		{
			throw ValueException::FromString("Incorrect arguments passed to addEventListener");
		}

		this->AddEventListener(event, callback);
		result->SetMethod(callback);
	}

	void KEventObject::_RemoveEventListener(const ValueList& args, KValueRef result)
	{
		args.VerifyException("removeEventListener", "s m");

		std::string event(args.GetString(0));
		this->RemoveEventListener(event, args.GetMethod(1));
	}

	void KEventObject::ReportDispatchError(std::string& reason)
	{
		this->logger()->Error("Failed to fire event: target=%s reason=%s",
			this->GetType().c_str(), reason.c_str());
	}


	EventListener::EventListener(std::string& targetedEvent, KMethodRef callback) :
		targetedEvent(targetedEvent),
		callback(callback)
	{
	}

	inline bool EventListener::Handles(std::string& event)
	{
		return targetedEvent == event || targetedEvent == Event::ALL;
	}

	inline KMethodRef EventListener::Callback()
	{
		return this->callback;
	}

	bool EventListener::Dispatch(KObjectRef thisObject, const ValueList& args, bool synchronous)
	{
		KValueRef result = RunOnMainThread(this->callback, thisObject, args, synchronous);
		if (result->IsBool())
			return result->ToBool();
		return true;
	}
}

