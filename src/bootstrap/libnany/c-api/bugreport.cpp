#include "nany/nany.h"
#include <yuni/io/file.h>
#include <yuni/core/math.h>
#include <yuni/core/process/program.h>
#include "libnany-config.h"
#include "common-debuginfo.hxx"
#include <string.h>

using namespace Yuni;




extern "C" char* nany_get_info_for_bugreport()
{
	try
	{
		Clob string;
		string << "> nanyc {c++/bootstrap} v" << nany_version();
		if (debugmode)
			string << " {debug}";
		string << '\n';

		string << "> compiled with ";
		nany_details_export_compiler_version(string);
		string << '\n';

		bool custombuild = Nany::Config::maxSymbolNameLength != 64
			or Nany::Config::maxNamespaceDepth != 32
			or Nany::Config::maxFuncDeclParameterCount != 7
			or Nany::Config::maxPushedParameters != 32;
		if (custombuild)
		{
			string << "> !! nany.build.config: "
				<< "pp:" << Nany::Config::maxPushedParameters
				<< ", p:" << Nany::Config::maxFuncDeclParameterCount
				<< ", nd:" << Nany::Config::maxNamespaceDepth
				<< ", sl:" << Nany::Config::maxSymbolNameLength
				<< '\n';
		}

		string << "> os: ";
		#ifdef YUNI_OS_LINUX
		{
			String distribName;
			if (IO::errNone == IO::File::LoadFromFile(distribName, "/etc/issue.net"))
			{
				distribName.replace("\n", " ");
				distribName.trim();
				if (not distribName.empty())
					string << distribName << ", ";
			}
			string << Process::System("uname -m -s -p -r") << ", ";
		}
		#endif
		nany_details_export_system(string);
		string << '\n';

		string << "> cpu: " << System::CPU::Count() << " cpu(s)/core(s)\n";
		#ifdef YUNI_OS_LINUX
		{
			auto cpus = Process::System("sh -c \"grep 'model name' /proc/cpuinfo | cut -d':' -f 2 |  sort -u\"");
			cpus.words("\n", [&](AnyString line) -> bool {
				line.trim();
				if (not line.empty())
					string << "> cpu: " << line << '\n';
				return true;
			});
		}
		#endif

		string << "> ";
		nany_details_export_memory_usage(string);
		string << '\n';

		char* result = (char*)::malloc(sizeof(char) * (string.sizeInBytes() + 1));
		memcpy(result, string.data(), string.sizeInBytes());
		result[string.sizeInBytes()] = '\0';
		return result;
	}
	catch (std::bad_alloc&)
	{
		return nullptr;
	}
	catch (...)
	{
		char* txt = (char*)::malloc(16);
		strncpy(txt, "<error>", 16);
		return txt;
	}
}
