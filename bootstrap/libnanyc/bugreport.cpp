#include <nanyc/library.h>
#include <yuni/core/math.h>
#include <yuni/core/process/program.h>
#include <yuni/core/string.h>
#include <yuni/core/system/cpu.h>
#include <yuni/core/system/memory.h>
#include <yuni/io/file.h>
#include "libnanyc-config.h"
#include "libnanyc-version.h"
#ifdef YUNI_OS_LINUX
#include <sys/utsname.h>
#endif

namespace {

void printnyVersion(yuni::String& out) {
	out << "> nanyc {c++/bootstrap} v" << LIBNANYC_VERSION_STR;
	if (yuni::debugmode)
		out << " {debug}";
	out << '\n';
}

void printCompiler(yuni::String& out) {
	out << "> compiled with " << YUNI_COMPILER_NAME;
	#ifdef __VERSION__
	{
		out << ' ' << __VERSION__;
	}
	#else
	{
		#ifdef _MSC_FULL_VER
		out << ' ' << _MSC_FULL_VER;
		#endif
		#ifdef __clang__
		out << ' ' << __clang_version__;
		#else
		# ifdef __GNUC__
		out << ' ' << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
		# endif
		#endif
		#ifdef __xlc__
		out << ' ' << __xlc__;
		#endif
	}
	#endif
	out << '\n';
}

void printBuildFlags(yuni::String& out) {
	out << "> config: ";
	out << "params:" << ny::config::maxFuncDeclParameterCount;
	out << ", pushedparams:" << ny::config::maxPushedParameters;
	out << ", nmspc depth:" << ny::config::maxNamespaceDepth;
	out << ", symbol:" << ny::config::maxSymbolNameLength;
	out << ", nsl:" << ny::config::importNSL;
	out << '\n';
}

void printOS(yuni::String& out) {
	out << "> os:  ";
	#ifdef YUNI_OS_LINUX
	{
		yuni::String distribName;
		if (yuni::IO::errNone == yuni::IO::File::LoadFromFile(distribName, "/etc/issue.net")) {
			distribName.replace('\n', ' ');
			distribName.trim();
			if (not distribName.empty())
				out << distribName << ", ";
		}
		struct utsname un;
		if (uname(&un) == 0)
			out << un.sysname << ' ' << un.release << " (" << un.machine << ')';
		else
			out << "(unknown linux)";
	}
	#else
	{
		out << YUNI_OS_NAME;
		#ifdef YUNI_OS_64
		out << " (64bits)";
		#else
		out << " (32bits)";
		#endif
	}
	#endif
	out << '\n';
}

void printCPU(yuni::String& out) {
	yuni::ShortString64 cpustr;
	cpustr << yuni::System::CPU::Count() << " cpu(s)/core(s)";
	bool cpuAdded = false;
	if (yuni::System::linux) {
		auto cpus = yuni::Process::System("sh -c \"grep 'model name' /proc/cpuinfo | cut -d':' -f 2 |  sort -u\"");
		cpus.words("\n", [&](AnyString line) -> bool {
			line.trim();
			if (not line.empty()) {
				out << "> cpu: " << line;
				if (not cpuAdded) {
					out << " (" << cpustr << ')';
					cpuAdded = true;
				}
				out << '\n';
			}
			return true;
		});
	}
	if (not cpuAdded)
		out << "> cpu: " << cpustr << '\n';
}

void printMemory(yuni::String& out) {
	out << "> ";
	yuni::System::Memory::Usage usage;
	auto total = usage.total / (1024u * 1024u * 1024u);
	auto avail = yuni::Math::Round((double) usage.available / (1024u * 1024u * 1024u), 2);
	yuni::ShortString64 availstr = avail;
	availstr.trimRight('0');
	out << "mem: " << availstr << " GiB free / " << total << " GiB";
	out << '\n';
}

} // namespace

char* libnanyc_get_bugreportdetails(uint32_t* length) {
	try {
		yuni::String report;
		report.reserve(512);
		printnyVersion(report);
		printCompiler(report);
		printBuildFlags(report);
		printOS(report);
		printCPU(report);
		printMemory(report);
		if (length)
			*length = report.size();
		return report.forgetContent();
	}
	catch (...) {
		if (length)
			*length = 0;
		return NULL;
	}
}
