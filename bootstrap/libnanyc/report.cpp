#include <nanyc/report.h>
#include "libnanyc.h"
#include "details/reporting/message.h"
#include <string.h>

#ifndef YUNI_OS_WINDOWS
#define SEP " \u205E "
#else
#define SEP " : "
#endif

namespace {

nyerrlevel_t toErrLevel(ny::Logs::Level level) {
	switch (level) {
		case ny::Logs::Level::ICE:     return nyel_ice;
		case ny::Logs::Level::error:   return nyel_error;
		case ny::Logs::Level::warning: return nyel_warning;
		case ny::Logs::Level::hint:    return nyel_hint;
		case ny::Logs::Level::suggest: return nyel_suggest;
		case ny::Logs::Level::success: return nyel_success;
		case ny::Logs::Level::info:    return nyel_info;
		case ny::Logs::Level::verbose:
		case ny::Logs::Level::trace:   return nyel_trace;
		case ny::Logs::Level::none:    return nyel_none;
	}
	return nyel_none;
}

struct Visitor final {
	void* userdata;
	const nyreport_visitor_opts_t& opts;

	Visitor(const nyreport_visitor_opts_t& opts)
		: userdata(opts.userdata)
		, opts(opts) {
	}

	void visit(const ny::Logs::Message& message, const nyreport_entry_t* parent, uint32_t depth) {
		nyreport_entry_t entry;
		memset(&entry, 0x0, sizeof(nyreport_entry_t));
		yuni::MutexLocker locker{message.m_mutex};
		entry.errlevel = toErrLevel(message.level);
		entry.highlight.c_str = message.prefix.c_str();
		entry.highlight.len = message.prefix.size();
		entry.text.c_str = message.message.c_str();
		entry.text.len = message.message.size();
		entry.origin.filename.c_str = message.origins.location.filename.c_str();
		entry.origin.filename.len = message.origins.location.filename.size();
		entry.origin.line = message.origins.location.pos.line;
		entry.origin.column = message.origins.location.pos.offset;
		entry.item_count = static_cast<uint32_t>(message.entries.size());
		entry.depth = depth;
		entry.parent = parent;
		if (opts.on_entry) {
			switch (opts.on_entry(userdata, &entry)) {
				case nyf_continue: break;
				case nyf_skip: return;
				default: throw false;
			}
		}
		if (entry.item_count != 0) {
			if (entry.errlevel != nyel_none)
				++depth;
			for (auto& ptr: message.entries)
				visit(*ptr, &entry, depth);
		}
		if (opts.on_entry_leave) {
			switch (opts.on_entry_leave(userdata, &entry)) {
				case nyf_continue: break;
				case nyf_skip: break;
				default: throw false;
			}
		}
	}

	nybool_t visit(const nyreport_t* report) {
		if (opts.on_start)
			userdata = opts.on_start(userdata, report);
		nybool_t result = nytrue;
		try {
			auto& message = *reinterpret_cast<const ny::Logs::Message*>(report);
			for (auto& ptr: message.entries)
				visit(*ptr, nullptr, 0);
		}
		catch (...) {
			result = nyfalse;
		}
		if (opts.on_end)
			result = opts.on_end(userdata, report, result);
		return result;
	}
};

} // namespace

nybool_t nyreport_visit(const nyreport_visitor_opts_t* opts, const nyreport_t* report) {
	if (likely(opts and report))
		return Visitor(*opts).visit(report);
	return nytrue;
}

nybool_t nyreport_print(const nyreport_t* report, nyconsole_t* console) {
	if (unlikely(!report or !console))
		return nyfalse;
	struct Printer final {
		nyconsole_t* console;
		nyreport_visitor_opts_t opts;
		yuni::String tmpstr;
	}
	printer;
	printer.console = console;
	memset(&printer.opts, 0x0, sizeof(nyreport_visitor_opts_t));
	printer.opts.userdata = &printer;
	printer.opts.on_entry = [](void* userdata, const nyreport_entry_t* entry) -> nyflow_t {
		Printer& printer = *reinterpret_cast<Printer*>(userdata);
		auto* const console = printer.console;
		auto print = [&](const AnyString& str) {
			console->write(console, str.c_str(), str.size());
		};
		auto printex = [&](const AnyString& str, nycolor_t color) {
			console->set_color(console, color);
			console->write(console, str.c_str(), str.size());
			console->set_color(console, nyc_default);
		};
		auto printanystr = [&](const nyanystr_t& str, nycolor_t color) {
			console->set_color(console, color);
			console->write(console, str.c_str, str.len);
			console->set_color(console, nyc_default);
		};
		switch (entry->errlevel) {
			case nyel_error: {
				printex("   error" SEP, nyc_red);
				break;
			}
			case nyel_warning: {
				printex(" warning" SEP, nyc_yellow);
				break;
			}
			case nyel_info: {
				print("        " SEP);
				break;
			}
			case nyel_hint:
			case nyel_suggest: {
				print("        " SEP);
				break;
			}
			case nyel_success: {
				#ifndef YUNI_OS_WINDOWS
				printex("      \u2713 " SEP, nyc_green);
				#else
				printex("      ok" SEP, nyc_green);
				#endif
				break;
			}
			case nyel_trace: {
				printex("      ::", nyc_magenta);
				print(SEP);
				break;
			}
			case nyel_ice: {
				printex("     ICE" SEP, nyc_red);
				break;
			}
			case nyel_none:
				return nyf_continue;
		}
		for (uint32_t i = entry->depth; i--; )
			print("    ");
		if (entry->highlight.len != 0)
			printanystr(entry->highlight, nyc_white);
		switch (entry->errlevel) {
			default: break;
			case nyel_suggest: printex("suggest: ", nyc_lightblue); break;
			case nyel_hint:    printex("hint: ", nyc_lightblue); break;
		}
		auto& origin = entry->origin;
		if (origin.filename.len != 0) {
			console->set_color(console, nyc_white);
			console->write(console, origin.filename.c_str, origin.filename.len);
			if (origin.line != 0) {
				yuni::ShortString16 xx;
				xx << ':' << origin.line;
				if (origin.column != 0)
					xx << ':' << origin.column;
				console->write(console, xx.c_str(), xx.size());
			}
			console->set_color(console, nyc_default);
			console->write(console, ": ", 2);
		}
		if (entry->text.len != 0) {
			auto& tmpstr = printer.tmpstr;
			tmpstr.assign(entry->text.c_str, static_cast<uint32_t>(entry->text.len));
			tmpstr.trimRight(" \t\r\n");
			tmpstr.replace("\t", "    "); // tabs
			auto firstLF = tmpstr.find('\n');
			if (not (firstLF < tmpstr.size())) {
				console->write(console, tmpstr.c_str(), tmpstr.size());
			}
			else {
				yuni::String s;
				s.clear() << "\n        " SEP;
				for (uint32_t i = entry->depth; i--; )
					s.write("    ", 4);
				bool addLF = false;
				tmpstr.words("\n", [&](const AnyString & word) -> bool {
					if (addLF)
						print(s);
					print(word);
					addLF = true;
					return true;
				});
			}
		}
		console->write(console, "\n", 1);
		return nyf_continue;
	};
	return Visitor(printer.opts).visit(report);
}

nybool_t nyreport_print_stdout(const nyreport_t* report) {
	nyconsole_t console;
	nyconsole_init_from_stdout(&console);
	return nyreport_print(report, &console);
}
