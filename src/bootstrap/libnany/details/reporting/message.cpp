#include "message.h"
#include <yuni/core/system/console/console.h>

using namespace Yuni;



namespace Nany
{
namespace Logs
{


	Message& Message::createEntry(Level level)
	{
		Message* entry = new Message{level};

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

		static void printMessage(nyconsole_cf_t& out, const Message& message, uint32_t indent, String& xx, bool unify)
		{
			#ifndef YUNI_OS_WINDOWS
			static const AnyString sep = " \u205E ";
			#else
			static const AnyString sep = " : ";
			#endif

			Message::ThreadingPolicy::MutexLocker locker{message};

			// which output ?
			nyconsole_output_t omode = (unify or (message.level != Level::error and message.level != Level::warning))
				? nycout : nycerr;
			// function writer
			auto wrfn = (omode == nycout) ? out.write_stdout : out.write_stderr;

			auto print = [&](const AnyString& text) {
				wrfn(out.internal, text.c_str(), text.size());
			};

			if (message.level != Level::none)
			{
				switch (message.level)
				{
					case Level::error:
					{
						out.set_color(out.internal, omode, nyc_red);
						#ifndef YUNI_OS_WINDOWS
						print("   error \u220E ");
						#else
						print("   error > ");
						#endif
						break;
					}
					case Level::warning:
					{
						out.set_color(out.internal, omode, nyc_yellow);
						print(" warning > ");
						break;
					}
					case Level::info:
					{
						if (message.section.empty())
						{
							out.set_color(out.internal, omode, nyc_none);
							print("        ");
							print(sep);
						}
						else
						{
							out.set_color(out.internal, omode, nyc_lightblue);
							xx = "        ";
							xx.overwriteRight(message.section);
							xx << sep;
							print(xx);
						}
						break;
					}

					case Level::hint:
					case Level::suggest:
					{
						out.set_color(out.internal, omode, nyc_none);
						print("        ");
						print(sep);
						break;
					}

					case Level::success:
					{
						out.set_color(out.internal, omode, nyc_green);
						#ifndef YUNI_OS_WINDOWS
						print("      \u2713 ");
						#else
						print("        ");
						#endif
						print(sep);
						break;
					}
					case Level::trace:
					{
						out.set_color(out.internal, omode, nyc_purple);
						print("      ::");
						out.set_color(out.internal, omode, nyc_none);
						print(sep);
						break;
					}
					case Level::verbose:
					{
						out.set_color(out.internal, omode, nyc_green);
						print("      ::");
						out.set_color(out.internal, omode, nyc_none);
						print(sep);
						break;
					}
					case Level::ICE:
					{
						out.set_color(out.internal, omode, nyc_red);
						print("     ICE");
						print(sep);
						break;
					}
					case Level::none:
					{
						break;
					}
				}

				for (uint32_t i = indent; i--; )
					print("    ");

				if (not message.prefix.empty())
				{
					out.set_color(out.internal, omode, nyc_white);
					print(message.prefix);
				}

				if (message.level == Level::suggest)
				{
					out.set_color(out.internal, omode, nyc_lightblue);
					print("suggest: ");
				}
				else if (message.level == Level::hint)
				{
					out.set_color(out.internal, omode, nyc_lightblue);
					print("hint: ");
				}

				if (not message.origins.location.target.empty())
				{
					out.set_color(out.internal, omode, nyc_none);
					print("{");
					print(message.origins.location.target);
					print("} ");
				}

				if (not message.origins.location.filename.empty())
				{
					out.set_color(out.internal, omode, nyc_white);
					xx = message.origins.location.filename;

					if (message.origins.location.pos.line > 0)
					{
						xx << ':';
						xx << message.origins.location.pos.line;
						if (message.origins.location.pos.offset != 0)
						{
							xx << ':';
							xx << message.origins.location.pos.offset;
							if (message.origins.location.pos.offsetEnd != 0)
							{
								xx << '-';
								xx << message.origins.location.pos.offsetEnd;
							}
						}
						print(xx);
					}
					xx << ": ";
					print(xx);
				}


				if (not message.message.empty())
				{
					out.set_color(out.internal, omode, nyc_none);

					String msg{message.message};
					msg.trimRight(" \t\r\n");
					msg.replace("\t", "    "); // tabs

					auto firstLF = msg.find('\n');
					if (firstLF < message.message.size())
					{
						xx.clear() <<"\n        " << sep;
						for (uint i = indent; i--; )
							xx.write("    ", 4);

						bool addLF = false;
						msg.words("\n", [&](const AnyString& word) -> bool
						{
							if (addLF)
								print(xx);
							print(word);
							addLF = true;
							return true;
						});
					}
					else
						print(msg);
				}

				print("\n");
				++indent;
			}

			for (auto& ptr: message.entries)
				printMessage(out, *ptr, indent, xx, unify);
		}


	} // anonymous namespace


	void Message::print(nyconsole_cf_t& out, bool unify)
	{
		String tmp;
		printMessage(out, *this, 0, tmp, unify);
	}




} // namespace Logs
} // namespace Nany
