#include "message.h"
#include <yuni/core/system/console/console.h>

using namespace Yuni;



namespace Nany
{
namespace Logs
{

	Message& Message::createEntry(Level level)
	{
		Message::Ptr entry = new Message(level);

		ThreadingPolicy::MutexLocker locker{*this};
		entry->origins = origins;
		entries.push_back(entry);
		return *entry;
	}


	void Message::appendEntry(const Message::Ptr& entry)
	{
		if (!!entry)
		{
			if (not (entry->entries.empty() and entry->level == Level::none))
			{
				ThreadingPolicy::MutexLocker locker{*this};
				entries.push_back(entry);
				hasErrors &= entry->hasErrors;
			}
		}
	}





	namespace // anonymous
	{

		template<class T>
		static void printMessage(T& out, const Message& message, uint indent, String& tmp)
		{
			#ifndef YUNI_OS_WINDOWS
			static const AnyString sep = " \u205E ";
			#else
			static const AnyString sep = " | ";
			#endif

			Message::ThreadingPolicy::MutexLocker locker{message};

			if (message.level != Level::none)
			{
				switch (message.level)
				{
					case Level::error:
					{
						System::Console::SetTextColor(out, System::Console::red);
						#ifndef YUNI_OS_WINDOWS
						out << "      \u2718 " << sep;
						#else
						out << "       X" << sep;
						#endif
						break;
					}
					case Level::warning:
					{
						System::Console::SetTextColor(out, System::Console::yellow);
						#ifndef YUNI_OS_WINDOWS
						out << "      \u26A0 " << sep;
						#else
						out << "       !" << sep;
						#endif
						break;
					}
					case Level::info:
					{
						if (message.section.empty())
						{
							System::Console::ResetTextColor(out);
							out << "        " << sep;
						}
						else
						{
							ShortString16 sect{"        "};
							sect.overwriteRight(message.section);
							System::Console::SetTextColor(out, System::Console::lightblue);
							out << sect;
							//System::Console::ResetTextColor(out);
							out << sep;
						}
						break;
					}

					case Level::hint:
					case Level::suggest:
					{
						System::Console::ResetTextColor(out);
						out << "        " << sep;
						break;
					}

					case Level::success:
					{
						System::Console::SetTextColor(out, System::Console::green);
						#ifndef YUNI_OS_WINDOWS
						out << "      \u2713 " << sep;
						#else
						out << "        " << sep;
						#endif
						break;
					}

					case Level::trace:
						System::Console::SetTextColor(out, System::Console::purple);
						out << "      ::";
						System::Console::ResetTextColor(out);
						out << sep;
						break;
					case Level::verbose:
						System::Console::SetTextColor(out, System::Console::green);
						out << "      ::";
						System::Console::ResetTextColor(out);
						out << sep;
						break;
					case Level::ICE:
						System::Console::SetTextColor(out, System::Console::red);
						out << "     ICE" << sep;
						break;

					case Level::none:
						break;
				}

				for (uint i = indent; i--; )
					out.write("    ", 4);

				if (not message.prefix.empty())
				{
					System::Console::SetTextColor(out, System::Console::white);
					out << message.prefix;
				}

				if (message.level == Level::suggest)
				{
					System::Console::SetTextColor(out, System::Console::lightblue);
					out << "suggest: ";
				}
				if (message.level == Level::hint)
				{
					System::Console::SetTextColor(out, System::Console::lightblue);
					out << "hint: ";
				}

				if (not message.origins.location.target.empty())
				{
					System::Console::ResetTextColor(out);
					out << '{' << message.origins.location.target;
					out.write("} ", 2);
				}

				if (not message.origins.location.filename.empty())
				{
					System::Console::SetTextColor(out, System::Console::white);
					out << message.origins.location.filename;

					if (message.origins.location.pos.line > 0)
					{
						out << ':';
						out << message.origins.location.pos.line;
						if (message.origins.location.pos.offset != 0)
						{
							out << ':' << message.origins.location.pos.offset;
							if (message.origins.location.pos.offsetEnd != 0)
								out << '-' << message.origins.location.pos.offsetEnd;
						}
					}
					out.write(": ", 2);
				}


				if (not message.message.empty())
				{
					System::Console::ResetTextColor(out);
					String msg{message.message};
					msg.trimRight(" \t\r\n");
					msg.replace("\t", "    "); // tabs

					auto firstLF = msg.find('\n');
					if (firstLF < message.message.size())
					{
						tmp.clear() <<"\n        " << sep;
						for (uint i = indent; i--; )
							tmp.write("    ", 4);

						bool addLF = false;
						msg.words("\n", [&](const AnyString& word) -> bool
						{
							if (addLF)
								out << tmp;
							out << word;
							addLF = true;
							return true;
						});
					}
					else
						out << msg;
				}

				out << '\n';
				++indent;
			}

			for (auto& ptr: message.entries)
				printMessage(out, *ptr, indent, tmp);
		}


	} // anonymous namespace




	void Message::print(std::ostream& out) const
	{
		String tmp;
		printMessage(out, *this, 0, tmp);
		out << std::flush;
	}


	void Message::print(Clob& out) const
	{
		String tmp;
		printMessage(out, *this, 0, tmp);
	}







} // namespace Logs
} // namespace Nany
