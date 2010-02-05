/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KROLL_OSX_NSLOG_CHANNEL_
#define _KROLL_OSX_NSLOG_CHANNEL_

#include <Poco/Channel.h>
#include <Poco/PatternFormatter.h>
#include "Poco/Mutex.h"
//#include "Poco/UnWindows.h"

namespace kroll
{
	class NSLogChannel : public Poco::Channel
	{
		public:
			NSLogChannel(); 
			void log(const Poco::Message& msg);

		protected:
			~NSLogChannel() {};

		private:
			Poco::PatternFormatter formatter;
	};
}
#endif 
