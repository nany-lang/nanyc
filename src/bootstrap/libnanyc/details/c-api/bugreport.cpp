#include "nany/nany.h"
#include <yuni/io/file.h>
#include <yuni/core/math.h>
#include <yuni/core/process/program.h>
#include "libnanyc-config.h"
#include "common-debuginfo.hxx"
#include <string.h>
#include <iostream>
#ifdef YUNI_OS_LINUX
#include <sys/utsname.h>
#endif

using namespace Yuni;



namespace // anonymous
{

	template<class T>
	static inline void buildBugReport(T& string)
	{
		string << "> nanyc {c++/bootstrap} v" << nylib_version();
		if (debugmode)
			string << " {debug}";
		string << '\n';

		string << "> compiled with ";
		nany_details_export_compiler_version(string);
		string << '\n';

		string << "> config: "
			<< "params:" << Nany::Config::maxFuncDeclParameterCount
			<< ", pushedparams:" << Nany::Config::maxPushedParameters
			<< ", nmspc depth:" << Nany::Config::maxNamespaceDepth
			<< ", symbol:" << Nany::Config::maxSymbolNameLength
			<< ", nsl:" << Nany::Config::importNSL
			<< '\n';

		// -- OS
		string << "> os:  ";
		bool osDetected = false;
		#ifdef YUNI_OS_LINUX
		{
			String distribName;
			if (IO::errNone == IO::File::LoadFromFile(distribName, "/etc/issue.net"))
			{
				distribName.replace('\n', ' ');
				distribName.trim();
				if (not distribName.empty())
					string << distribName << ", ";
			}

			osDetected = true;
			struct utsname un;
			if (uname(&un) == 0)
				string << un.sysname << ' ' << un.release << " (" << un.machine << ')';
			else
				string << "(unknown linux)";
		}
		#endif

		if (not osDetected)
			nany_details_export_system(string);
		string << '\n';

		// -- CPU
		ShortString64 cpustr;
		cpustr << System::CPU::Count() << " cpu(s)/core(s)";

		bool cpuAdded = false;
		if (System::linux)
		{
			auto cpus = Process::System("sh -c \"grep 'model name' /proc/cpuinfo | cut -d':' -f 2 |  sort -u\"");
			cpus.words("\n", [&](AnyString line) -> bool
			{
				line.trim();
				if (not line.empty())
				{
					string << "> cpu: " << line;
					if (not cpuAdded)
					{
						string << " (" << cpustr << ')';
						cpuAdded = true;
					}
					string << '\n';
				}
				return true;
			});
		}

		if (not cpuAdded)
			string << "> cpu: " << cpustr << '\n';

		// -- MEMORY
		string << "> ";
		nany_details_export_memory_usage(string);
		string << '\n';
	}

} // anonymous namespace




extern "C" void nylib_print_info_for_bugreport()
{
	buildBugReport(std::cout);
}


extern "C" char* nylib_get_info_for_bugreport(uint32_t* length)
{
	String string;
	string.reserve(512);
	buildBugReport(string);

	char* result = (char*)::malloc(sizeof(char) * (string.sizeInBytes() + 1));
	if (!result)
	{
		if (length)
			*length = 0u;
		return nullptr;
	}
	memcpy(result, string.data(), string.sizeInBytes());
	result[string.sizeInBytes()] = '\0';
	if (length)
		*length = string.size();
	return result;
}
