#include "../kroll.h"

namespace kroll
{
	NSLogChannel::NSLogChannel()
	{
		this->formatter = PatternFormatter("[%s] [%p] %t");
	}

	void NSLogChannel::log(const Poco::Message& msg)
	{
		std::string text;
		formatter.format(msg, text);
		NSLog(@"%s", text.c_str());
	}
}
