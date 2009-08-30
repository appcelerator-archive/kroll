/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
namespace kroll
{
	std::string Event::ALL = "all";
	std::string Event::FOCUSED = "focused";
	std::string Event::UNFOCUSED = "unfocused";
	std::string Event::OPEN = "open";
	std::string Event::OPENED = "opened";
	std::string Event::CLOSE = "close";
	std::string Event::CLOSED = "closed";
	std::string Event::HIDDEN = "hidden";
	std::string Event::SHOWN = "shown";
	std::string Event::FULLSCREENED = "fullscreened";
	std::string Event::UNFULLSCREENED = "unfullscreened";
	std::string Event::MAXIMIZED = "maximized";
	std::string Event::MINIMIZED = "minimized";
	std::string Event::RESIZED = "resized";
	std::string Event::MOVED = "moved";
	std::string Event::PAGE_INITIALIZED = "page.init";
	std::string Event::PAGE_LOADED = "page.load";
	std::string Event::CREATED = "create";
	std::string Event::ACTIVATE = "activate";
	std::string Event::CLICKED = "clicked";
	std::string Event::DOUBLE_CLICKED = "double.clicked";
	std::string Event::EXIT = "exit";
	std::string Event::READ = "read";

	Event::Event(AutoPtr<KEventObject> target, std::string& eventName) :
		AccessorBoundObject("Event"),
		target(target),
		eventName(eventName),
		stopped(false)
	{
		Event::SetEventConstants(this);
		this->SetMethod("getTarget", &Event::_GetTarget);
		this->SetMethod("getType", &Event::_GetType);
		this->SetMethod("getTimestamp", &Event::_GetTimestamp);
		this->SetMethod("stopPropagation", &Event::_StopPropagation);
	}

	void Event::_GetTarget(const ValueList&, SharedValue result)
	{
		result->SetObject(this->target);
	}

	void Event::_GetType(const ValueList&, SharedValue result)
	{
		result->SetString(this->eventName);
	}

	void Event::_GetTimestamp(const ValueList&, SharedValue result)
	{
		result->SetDouble((int) timestamp.epochMicroseconds() / 1000);
	}

	void Event::_StopPropagation(const ValueList&, SharedValue result)
	{
		this->stopped = true;
	}

	void Event::SetEventConstants(KObject* target)
	{
		// @tiproperty[String, ALL, since=0.6] The ALL event constant
		// @tiproperty[String, FOCUSED, since=0.6] The FOCUSED event constant
		// @tiproperty[String, UNFOCUSED, since=0.6] The UNFOCUSED event constant
		// @tiproperty[String, OPEN, since=0.6] The OPEN event constant
		// @tiproperty[String, OPENED, since=0.6] The OPENED event constant
		// @tiproperty[String, CLOSE, since=0.6] The CLOSE event constant
		// @tiproperty[String, CLOSED, since=0.6] The CLOSED event constant
		// @tiproperty[String, HIDDEN, since=0.6] The HIDDEN event constant
		// @tiproperty[String, SHOWN, since=0.6] The SHOWN event constant
		// @tiproperty[String, FULLSCREENED, since=0.6] The FULLSCREENED event constant
		// @tiproperty[String, UNFULLSCREENED, since=0.6] The UNFULLSCREENED event constant
		// @tiproperty[String, MAXIMIZED, since=0.6] The MAXIMIZED event constant
		// @tiproperty[String, MINIMIZED, since=0.6] The MINIMIZED event constant
		// @tiproperty[String, RESIZED, since=0.6] The RESIZED event constant
		// @tiproperty[String, MOVED, since=0.6] The MOVED event constant
		// @tiproperty[String, PAGE_INITIALIZED, since=0.6] The PAGE_INITIALIZED event constant
		// @tiproperty[String, PAGE_LOADED, since=0.6] The PAGE_LOADED event constant
		// @tiproperty[String, CREATE, since=0.6] The CREATE event constant
		// @tiproperty[String, EXIT, since=0.6] The EXIT event constant
		// @tiproperty[String, READ, since=0.6] The READ event constant

		target->Set("ALL", Value::NewString(Event::ALL));
		target->Set("FOCUSED", Value::NewString(Event::FOCUSED));
		target->Set("UNFOCUSED", Value::NewString(Event::UNFOCUSED));
		target->Set("OPEN", Value::NewString(Event::OPEN));
		target->Set("OPENED", Value::NewString(Event::OPENED));
		target->Set("CLOSE", Value::NewString(Event::CLOSE));
		target->Set("CLOSED", Value::NewString(Event::CLOSED));
		target->Set("HIDDEN", Value::NewString(Event::HIDDEN));
		target->Set("SHOWN", Value::NewString(Event::SHOWN));
		target->Set("FULLSCREENED", Value::NewString(Event::FULLSCREENED));
		target->Set("UNFULLSCREENED", Value::NewString(Event::UNFULLSCREENED));
		target->Set("MAXIMIZED", Value::NewString(Event::MAXIMIZED));
		target->Set("MINIMIZED", Value::NewString(Event::MINIMIZED));
		target->Set("RESIZED", Value::NewString(Event::RESIZED));
		target->Set("MOVED", Value::NewString(Event::MOVED));
		target->Set("PAGE_INITIALIZED", Value::NewString(Event::PAGE_INITIALIZED));
		target->Set("PAGE_LOADED", Value::NewString(Event::PAGE_LOADED));
		target->Set("CREATED", Value::NewString(Event::CREATED));
		target->Set("EXIT", Value::NewString(Event::EXIT));
		target->Set("READ", Value::NewString(Event::READ));
	}

}

