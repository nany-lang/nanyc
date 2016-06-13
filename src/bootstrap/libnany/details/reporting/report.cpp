#include "report.h"

using namespace Yuni;




namespace Nany
{
namespace Logs
{

	Report Report::fromErrLevel(Level level)
	{
		switch (level)
		{
			case Level::error:
			case Level::ICE:
				message.hasErrors = true;
			default: break;
		}
		return Report{message.createEntry(level)};
	}


	Report Report::ice()
	{
		message.hasErrors = true;
		return Report{message.createEntry(Level::ICE)} << "ICE: ";
	}

	Report Report::error()
	{
		message.hasErrors = true;
		return Report{message.createEntry(Level::error)};
	}

	Report Report::warning()
	{
		message.hasErrors = true;
		return Report{message.createEntry(Level::warning)};
	}

	Report Report::hint()
	{
		return Report{message.createEntry(Level::hint)};
	}

	Report Report::suggest()
	{
		return Report{message.createEntry(Level::suggest)};
	}

	Report Report::success()
	{
		return Report{message.createEntry(Level::success)};
	}

	Report Report::info()
	{
		return Report{message.createEntry(Level::info)};
	}

	Report Report::info(AnyString prefix)
	{
		auto& newmsg = message.createEntry(Level::info);
		newmsg.prefix = prefix;
		return Report{newmsg};
	}


	Report Report::verbose()
	{
		return Report{message.createEntry(Level::verbose)};
	}

	Report Report::trace()
	{
		return Report{message.createEntry(Level::trace)};
	}

	Report Report::subgroup()
	{
		return Report{message.createEntry(Level::none)};
	}




} // namespace Logs
} // namespace Nany
