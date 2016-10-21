#include "message.h"
#include "details/atom/atom.h"
#include <yuni/core/system/console/console.h>

using namespace Yuni;

#ifndef YUNI_OS_WINDOWS
#define SEP " \u205E "
#else
#define SEP " : "
#endif



namespace Nany
{
namespace Logs
{

	namespace {


	template<bool unify>
	void printMessageRecursive(nyconsole_t& out, const Message& message, String& xx, uint32_t indent = 0)
	{
		Message::ThreadingPolicy::MutexLocker locker{message};

		// which output ?
		nyconsole_output_t omode = (unify or (message.level != Level::error and message.level != Level::warning))
			? nycout : nycerr;
		// function writer
		auto wrfn = (omode == nycout) ? out.write_stdout : out.write_stderr;
		// print method
		auto print = [&](const AnyString& s) { wrfn(out.internal, s.c_str(), s.size()); };

		if (message.level != Level::none)
		{
			switch (message.level)
			{
				case Level::error:
				{
					out.set_color(out.internal, omode, nyc_red);
					print("   error" SEP);
					out.set_color(out.internal, omode, nyc_none);
					break;
				}
				case Level::warning:
				{
					out.set_color(out.internal, omode, nyc_yellow);
					print(" warning" SEP);
					out.set_color(out.internal, omode, nyc_none);
					break;
				}
				case Level::info:
				{
					if (message.section.empty())
					{
						out.set_color(out.internal, omode, nyc_none);
						print("        " SEP);
					}
					else
					{
						out.set_color(out.internal, omode, nyc_lightblue);
						xx = "        ";
						xx.overwriteRight(message.section);
						xx << SEP;
						print(xx);
						out.set_color(out.internal, omode, nyc_none);
					}
					break;
				}
				case Level::hint:
				case Level::suggest:
				{
					out.set_color(out.internal, omode, nyc_none);
					print("        " SEP);
					out.set_color(out.internal, omode, nyc_none);
					break;
				}
				case Level::success:
				{
					out.set_color(out.internal, omode, nyc_green);
					#ifndef YUNI_OS_WINDOWS
					print("      \u2713 " SEP);
					#else
					print("      ok" SEP);
					#endif
					out.set_color(out.internal, omode, nyc_none);
					break;
				}
				case Level::trace:
				{
					out.set_color(out.internal, omode, nyc_purple);
					print("      ::");
					out.set_color(out.internal, omode, nyc_none);
					print(SEP);
					break;
				}
				case Level::verbose:
				{
					out.set_color(out.internal, omode, nyc_green);
					print("      ::");
					out.set_color(out.internal, omode, nyc_none);
					print(SEP);
					break;
				}
				case Level::ICE:
				{
					out.set_color(out.internal, omode, nyc_red);
					print("     ICE" SEP);
					out.set_color(out.internal, omode, nyc_none);
					break;
				}
				case Level::none:
				{
					// unreachable - cf condition above
					break;
				}
			}

			if (indent)
			{
				xx.clear();
				for (uint32_t i = indent; i--; )
					xx << "    ";
				print(xx);
			}

			if (not message.prefix.empty())
			{
				out.set_color(out.internal, omode, nyc_white);
				print(message.prefix);
				out.set_color(out.internal, omode, nyc_none);
			}

			if (message.level == Level::suggest)
			{
				out.set_color(out.internal, omode, nyc_lightblue);
				print("suggest: ");
				out.set_color(out.internal, omode, nyc_none);
			}
			else if (message.level == Level::hint)
			{
				out.set_color(out.internal, omode, nyc_lightblue);
				print("hint: ");
				out.set_color(out.internal, omode, nyc_none);
			}

			if (not message.origins.location.target.empty())
			{
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
				}
				xx << ": ";
				print(xx);
				out.set_color(out.internal, omode, nyc_none);
			}

			if (not message.message.empty())
			{
				String msg{message.message};
				msg.trimRight(" \t\r\n");
				msg.replace("\t", "    "); // tabs

				auto firstLF = msg.find('\n');
				if (not (firstLF < message.message.size()))
				{
					print(msg);
				}
				else
				{
					xx.clear() << "\n        " SEP;
					for (uint i = indent; i--; )
						xx.write("    ", 4);

					bool addLF = false;
					msg.words("\n", [&](const AnyString& word) -> bool {
						if (addLF)
							print(xx);
						print(word);
						addLF = true;
						return true;
					});
				}
			}

			print("\n");
			++indent;
		}

		for (auto& ptr: message.entries)
			printMessageRecursive<unify>(out, *ptr, xx, indent);
	}


	} // anonymous namespace




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


	void Message::print(nyconsole_t& out, bool unify)
	{
		assert(out.set_color);
		if (out.set_color and out.write_stderr and out.write_stdout)
		{
			String tmp;
			tmp.reserve(1024);
			if (unify)
				printMessageRecursive<true>(out, *this, tmp);
			else
				printMessageRecursive<false>(out, *this, tmp);
		}
	}


	void Message::Origin::Location::resetFromAtom(const Atom& atom)
	{
		filename = atom.origin.filename;
		pos.line = atom.origin.line;
		pos.offset = atom.origin.offset;
	}




} // namespace Logs
} // namespace Nany
