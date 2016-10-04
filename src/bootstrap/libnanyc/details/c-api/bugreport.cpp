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

	void printNanyVersion(String& out)
	{
		out << "> nanyc {c++/bootstrap} v" << nylib_version();
		if (debugmode)
			out << " {debug}";
		out << '\n';
	}


	void printCompiler(String& out)
	{
		out << "> compiled with ";
		nany_details_export_compiler_version(out);
		out << '\n';
	}


	void printBuildFlags(String& out)
	{
		out << "> config: ";
		out << "params:" << Nany::Config::maxFuncDeclParameterCount;
		out << ", pushedparams:" << Nany::Config::maxPushedParameters;
		out << ", nmspc depth:" << Nany::Config::maxNamespaceDepth;
		out << ", symbol:" << Nany::Config::maxSymbolNameLength;
		out << ", nsl:" << Nany::Config::importNSL;
		out << '\n';
	}


	void printOS(String& out)
	{
		out << "> os:  ";
		bool osDetected = false;
		#ifdef YUNI_OS_LINUX
		{
			String distribName;
			if (IO::errNone == IO::File::LoadFromFile(distribName, "/etc/issue.net"))
			{
				distribName.replace('\n', ' ');
				distribName.trim();
				if (not distribName.empty())
					out << distribName << ", ";
			}

			osDetected = true;
			struct utsname un;
			if (uname(&un) == 0)
				out << un.sysname << ' ' << un.release << " (" << un.machine << ')';
			else
				out << "(unknown linux)";
		}
		#endif

		if (not osDetected)
			nany_details_export_system(out);
		out << '\n';
	}


	void printCPU(String& out)
	{
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
					out << "> cpu: " << line;
					if (not cpuAdded)
					{
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


	void printMemory(String& out)
	{
		out << "> ";
		nany_details_export_memory_usage(out);
		out << '\n';
	}


	void buildBugReport(String& out)
	{
		printNanyVersion(out);
		printCompiler(out);
		printBuildFlags(out);
		printOS(out);
		printCPU(out);
		printMemory(out);
	}


} // anonymous namespace




extern "C" void nylib_print_info_for_bugreport()
{
	String out;
	buildBugReport(out);
	std::cout << out;
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
