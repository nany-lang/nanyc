#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/project.h"
#include "details/context/build.h"
#include "libnanyc-version.h"

using namespace Yuni;

#ifndef YUNI_OS_WINDOWS
#define SEP " \u205E "
#else
#define SEP " : "
#endif


namespace { // anonymous

namespace {

void printMessageRecursive(nyoldconsole_t& out, const ny::Logs::Message& message, bool unify, String& xx,
		uint32_t indent = 0) {
	MutexLocker locker{message.m_mutex};
	// which output ?
	nyconsole_output_t omode = (unify or (message.level != ny::Logs::Level::error and message.level != ny::Logs::Level::warning))
		? nycout : nycerr;
	// function writer
	auto wrfn = (omode == nycout) ? out.write_stdout : out.write_stderr;
	// print method
	auto print = [&](const AnyString & s) {
		wrfn(out.internal, s.c_str(), s.size());
	};
	if (message.level != ny::Logs::Level::none) {
		switch (message.level) {
			case ny::Logs::Level::error: {
				out.set_color(out.internal, omode, nycold_red);
				print("   error" SEP);
				out.set_color(out.internal, omode, nycold_none);
				break;
			}
			case ny::Logs::Level::warning: {
				out.set_color(out.internal, omode, nycold_yellow);
				print(" warning" SEP);
				out.set_color(out.internal, omode, nycold_none);
				break;
			}
			case ny::Logs::Level::info: {
				if (message.section.empty()) {
					out.set_color(out.internal, omode, nycold_none);
					print("        " SEP);
				}
				else {
					out.set_color(out.internal, omode, nycold_lightblue);
					xx = "        ";
					xx.overwriteRight(message.section);
					xx << SEP;
					print(xx);
					out.set_color(out.internal, omode, nycold_none);
				}
				break;
			}
			case ny::Logs::Level::hint:
			case ny::Logs::Level::suggest: {
				out.set_color(out.internal, omode, nycold_none);
				print("        " SEP);
				out.set_color(out.internal, omode, nycold_none);
				break;
			}
			case ny::Logs::Level::success: {
				out.set_color(out.internal, omode, nycold_green);
				#ifndef YUNI_OS_WINDOWS
				print("      \u2713 " SEP);
				#else
				print("      ok" SEP);
				#endif
				out.set_color(out.internal, omode, nycold_none);
				break;
			}
			case ny::Logs::Level::trace: {
				out.set_color(out.internal, omode, nycold_purple);
				print("      ::");
				out.set_color(out.internal, omode, nycold_none);
				print(SEP);
				break;
			}
			case ny::Logs::Level::verbose: {
				out.set_color(out.internal, omode, nycold_green);
				print("      ::");
				out.set_color(out.internal, omode, nycold_none);
				print(SEP);
				break;
			}
			case ny::Logs::Level::ICE: {
				out.set_color(out.internal, omode, nycold_red);
				print("     ICE" SEP);
				out.set_color(out.internal, omode, nycold_none);
				break;
			}
			case ny::Logs::Level::none: {
				// unreachable - cf condition above
				break;
			}
		}
		if (indent) {
			xx.clear();
			for (uint32_t i = indent; i--; )
				xx << "    ";
			print(xx);
		}
		if (not message.prefix.empty()) {
			out.set_color(out.internal, omode, nycold_white);
			print(message.prefix);
			out.set_color(out.internal, omode, nycold_none);
		}
		if (message.level == ny::Logs::Level::suggest) {
			out.set_color(out.internal, omode, nycold_lightblue);
			print("suggest: ");
			out.set_color(out.internal, omode, nycold_none);
		}
		else if (message.level == ny::Logs::Level::hint) {
			out.set_color(out.internal, omode, nycold_lightblue);
			print("hint: ");
			out.set_color(out.internal, omode, nycold_none);
		}
		if (not message.origins.location.target.empty()) {
			print("{");
			print(message.origins.location.target);
			print("} ");
		}
		if (not message.origins.location.filename.empty()) {
			out.set_color(out.internal, omode, nycold_white);
			xx = message.origins.location.filename;
			if (message.origins.location.pos.line > 0) {
				xx << ':';
				xx << message.origins.location.pos.line;
				if (message.origins.location.pos.offset != 0) {
					xx << ':';
					xx << message.origins.location.pos.offset;
					if (message.origins.location.pos.offsetEnd != 0) {
						xx << '-';
						xx << message.origins.location.pos.offsetEnd;
					}
				}
			}
			xx << ": ";
			print(xx);
			out.set_color(out.internal, omode, nycold_none);
		}
		if (not message.message.empty()) {
			String msg{message.message};
			msg.trimRight(" \t\r\n");
			msg.replace("\t", "    "); // tabs
			auto firstLF = msg.find('\n');
			if (not (firstLF < message.message.size()))
				print(msg);
			else {
				xx.clear() << "\n        " SEP;
				for (uint i = indent; i--; )
					xx.write("    ", 4);
				bool addLF = false;
				msg.words("\n", [&](const AnyString & word) -> bool {
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
	for (auto& ptr : message.entries)
		printMessageRecursive(out, *ptr, unify, xx, indent);
}

void print(ny::Logs::Message& message, nyoldconsole_t& out, bool unify) {
	assert(out.set_color);
	if (out.set_color and out.write_stderr and out.write_stdout) {
		String tmp;
		tmp.reserve(1024);
		printMessageRecursive(out, message, unify, tmp);
	}
}

} // namespace

void nybuild_print_compiler_info_to_console(ny::Build& build) {
	// nanyc {c++/bootstrap} v0.1.0-alpha+ed25d59 {debug}
	{
		ny::Logs::Message msg{ny::Logs::Level::info};
		msg.section = "comp";
		msg.prefix = "nanyc {c++/bootstrap} ";
		msg.message << 'v' << LIBNANYC_VERSION_STR;
		if (debugmode)
			msg.message << " {debug}";
		print(msg, build.cf.console, false);
	}
}


} // anonymous namespace


extern "C" void nybuild_print_report_to_console(nybuild_t* ptr, nybool_t print_header) {
	if (ptr) {
		auto& build = ny::ref(ptr);
		try {
			if (YUNI_UNLIKELY(print_header != nyfalse))
				nybuild_print_compiler_info_to_console(build);
			print(build.compdb.messages, build.cf.console, false);
		}
		catch (...) {}
	}
}


extern "C" void nybuild_cf_init(nybuild_cf_t* cf) {
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nybuild_cf_t));
	nyconsole_cf_set_stdcout(&cf->console);
	// default entrypoint
	cf->entrypoint.len  = 4;
	cf->entrypoint.c_str = "main";
}


extern "C" nybuild_t* nybuild_prepare(nyproject_t* ptr, const nybuild_cf_t* cf) {
	if (ptr) {
		constexpr bool async = false;
		auto& project = ny::ref(ptr);
		std::unique_ptr<ny::Build> build;
		try {
			if (cf) {
				if (cf->on_query and (nyfalse == cf->on_query(ptr)))
					return nullptr;
				build = std::make_unique<ny::Build>(&project, *cf, async);
			}
			else {
				nybuild_cf_t ncf;
				nybuild_cf_init(&ncf);
				build = std::make_unique<ny::Build>(&project, ncf, async);
			}
			// making sure that user-events do not destroy the project by mistake
			build->addRef();
			return build.release()->self();
		}
		catch (...) {
		}
	}
	return nullptr;
}


extern "C" nybool_t nybuild(nybuild_t* ptr) {
	return ((ptr) ? ny::ref(ptr).compile() : false) ? nytrue : nyfalse;
}


extern "C" void nybuild_ref(nybuild_t* build) {
	if (build)
		ny::ref(build).addRef();
}


extern "C" void nybuild_unref(nybuild_t* ptr) {
	if (ptr) {
		try {
			auto& build = ny::ref(ptr);
			if (build.release())
				delete &build;
		}
		catch (...) {}
	}
}
