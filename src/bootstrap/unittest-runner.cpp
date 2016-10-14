#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <nany/nany.h>
#include <yuni/datetime/timestamp.h>
#include <yuni/core/system/cpu.h>
#include <yuni/core/system/console/console.h>
#include <yuni/io/file.h>
#include <set>
#include <iostream>
#include <cassert>
#include <random>
#include <chrono>

using namespace Yuni;


namespace
{

	static uint32_t jobCount = 0;
	static bool hasColorsOut = false;
	static bool hasColorsErr = false;


	int printBugReportInfo()
	{
		nylib_print_info_for_bugreport();
		return EXIT_SUCCESS;
	}


	int printVersion()
	{
		std::cout << nylib_version() << '\n';
		return EXIT_SUCCESS;
	}


	int printNoInputScript(const char* argv0)
	{
		std::cerr << argv0 << ": no input script file\n";
		return EXIT_FAILURE;
	}


	void fetchUnittestList(nyrun_cf_t& runcf, std::vector<String>& torun, const char** filelist, uint32_t count)
	{
		std::cout << "searching for unittests in all source files...\n" << std::flush;
		runcf.build.entrypoint.size = 0; // disable any compilation by default
		runcf.build.entrypoint.c_str = nullptr;
		runcf.program.entrypoint.size = 0;
		runcf.program.entrypoint.c_str = nullptr;
		runcf.build.ignore_atoms = nytrue;
		runcf.build.userdata = &torun;
		runcf.build.on_unittest = [](void* userdata, const char* mod, uint32_t modlen, const char* name, uint32_t nlen)
		{
			AnyString module{mod, modlen};
			AnyString testname{name, nlen};
			((std::vector<String>*) userdata)->emplace_back();
			auto& element = ((std::vector<String>*) userdata)->back();
			element << module << ':' << testname;
		};
		int64_t starttime = DateTime::NowMilliSeconds();
		nyrun_filelist(&runcf, filelist, count, 0, nullptr);
		int64_t duration = DateTime::NowMilliSeconds() - starttime;

		switch (torun.size())
		{
			case 0: std::cout << "0 test found"; break;
			case 1: std::cout << "1 test found"; break;
			default: std::cout << torun.size() << " tests found"; break;
		}
		std::cout << " (" << duration << "ms)\n";
	}


	int listAllUnittests(nyrun_cf_t& runcf, const char** filelist, uint32_t count)
	{
		std::set<std::pair<String, String>> alltests;

		runcf.build.entrypoint.size  = 0; // disable any compilation by default
		runcf.build.entrypoint.c_str = nullptr;
		runcf.program.entrypoint.size = 0;
		runcf.program.entrypoint.c_str = nullptr;
		runcf.build.ignore_atoms = nytrue;
		runcf.build.userdata = &alltests;
		runcf.build.on_unittest = [](void* userdata, const char* mod, uint32_t modlen, const char* name, uint32_t nlen)
		{
			AnyString module{mod, modlen};
			AnyString testname{name, nlen};
			auto& dict = *((std::set<std::pair<String, String>>*) userdata);
			dict.emplace(std::make_pair(String{module}, String{testname}));
		};

		int64_t starttime = DateTime::NowMilliSeconds();
		if (0 != nyrun_filelist(&runcf, filelist, count, 0, nullptr))
		{
			std::cerr << "error: failed to compile. aborting.\n";
			return EXIT_FAILURE;
		}
		int64_t duration = DateTime::NowMilliSeconds() - starttime;

		AnyString lastmodule;
		uint32_t moduleCount = 0;
		for (auto& pair: alltests)
		{
			auto& module = pair.first;
			if (lastmodule != module)
			{
				++moduleCount;
				lastmodule = module;
				if (hasColorsOut)
					System::Console::SetTextColor(std::cout, System::Console::bold);
				std::cout << '.' << lastmodule << ":\n";
				if (hasColorsOut)
					System::Console::ResetTextColor(std::cout);
			}

			std::cout << "       ";
			if (not module.empty())
				std::cout << module << ':';
			std::cout << pair.second << '\n';
		}

		std::cout << "\n       ";
		if (hasColorsOut)
			System::Console::SetTextColor(std::cout, System::Console::lightblue);
		switch (moduleCount)
		{
			case 0:  break;
			case 1:  std::cout << "1 module, "; break;
			default: std::cout << moduleCount << " modules, "; break;
		}
		switch (alltests.size())
		{
			case 0:  std::cout << "0 test found"; break;
			case 1:  std::cout << "1 test found"; break;
			default: std::cout << alltests.size() << " tests found"; break;
		}
		if (hasColorsOut)
			System::Console::ResetTextColor(std::cout);
		std::cout << "  (" << duration << "ms)\n\n";
		return EXIT_SUCCESS;
	}


	template<bool Fancy>
	bool runtest(nyrun_cf_t& originalRuncf, const String& testname, const char** filelist, uint32_t count)
	{
		ShortString256 entry;
		entry << "^unittest^" << testname;
		entry.replace("<nomodule>", "module");
		bool interactive = (hasColorsOut and jobCount == 1);

		auto runcf = originalRuncf;
		runcf.build.entrypoint.size  = entry.size();
		runcf.build.entrypoint.c_str = entry.c_str();
		runcf.program.entrypoint = runcf.build.entrypoint;
		runcf.build.ignore_atoms = nyfalse;

		struct Console
		{
			Clob cout;
			Clob cerr;
		}
		console;

		runcf.console.release = nullptr;
		runcf.console.internal = &console;
		runcf.console.flush = [](void*, nyconsole_output_t) {};
		runcf.console.set_color = [](void*, nyconsole_output_t, nycolor_t) {};
		runcf.console.has_color = [](void*, nyconsole_output_t) { return nyfalse; };

		runcf.console.write_stdout = [](void* userdata, const char* text, size_t length)
		{
			auto& console = *((Console*) userdata);
			if (text and length)
				console.cout.append(text, static_cast<uint32_t>(length));
		};
		runcf.console.write_stderr = [](void* userdata, const char* text, size_t length)
		{
			auto& console = *((Console*) userdata);
			if (text and length)
				console.cerr.append(text, static_cast<uint32_t>(length));
		};

		if (interactive and Fancy)
		{
			if (hasColorsOut)
				System::Console::SetTextColor(std::cout, System::Console::bold);
			std::cout << "       running ";
			if (hasColorsOut)
				System::Console::ResetTextColor(std::cout);
			std::cout << testname << "... " << std::flush;
		}

		int64_t starttime = DateTime::NowMilliSeconds();

		bool success = !nyrun_filelist(&runcf, filelist, count, 0, nullptr);
		int64_t duration = DateTime::NowMilliSeconds() - starttime;

		success &= console.cerr.empty();

		if (Fancy)
		{
			if (interactive)
				std::cout << '\r';

			if (success)
			{
				if (hasColorsOut)
					System::Console::SetTextColor(std::cout, System::Console::green);
				#ifndef YUNI_OS_WINDOWS
				std::cout << "    \u2713  ";
				#else
				std::cout << "   OK  ";
				#endif
			}
			else
			{
				if (hasColorsOut)
					System::Console::SetTextColor(std::cout, System::Console::red);
				std::cout << "  ERR  ";
			}

			if (hasColorsOut)
				System::Console::ResetTextColor(std::cout);

			std::cout << testname;

			if (duration < 1200 or not hasColorsOut)
			{
				std::cout << "  (" << duration << "ms)     ";
			}
			else
			{
				System::Console::SetTextColor(std::cout, System::Console::purple);
				std::cout << "  (" << duration << "ms)     ";
				System::Console::ResetTextColor(std::cout);
			}
			std::cout << '\n';

			if (not console.cerr.empty())
			{
				console.cerr.trimRight();
				console.cerr.replace("\n", "\n       | ");
				std::cout << "       | " << console.cerr << '\n';
			}

			std::cout << std::flush;
		}
		return success;
	}


	void printStatstics(const std::vector<String>& optToRun, int64_t duration, uint32_t successCount, uint32_t failCount)
	{
		switch (optToRun.size())
		{
			case 0:  std::cout << "\n       0 test, "; break;
			case 1:  std::cout << "\n       1 test"; break;
			default: std::cout << "\n       " << optToRun.size() << " tests, "; break;
		}
		if (successCount and hasColorsOut)
			System::Console::SetTextColor(std::cout, System::Console::green);
		switch (successCount)
		{
			case 0:  std::cout << "0 passing"; break;
			case 1:  std::cout << "1 passing"; break;
			default: std::cout << successCount << " passing"; break;
		}
		if (successCount and hasColorsOut)
			System::Console::ResetTextColor(std::cout);
		if (failCount)
			std::cout << ", ";

		if (failCount and hasColorsOut)
			System::Console::SetTextColor(std::cout, System::Console::red);
		switch (failCount)
		{
			case 0:  break;
			case 1:  std::cout << "1 failed"; break;
			default: std::cout << "" << failCount << " failed"; break;
		}
		if (failCount and hasColorsOut)
			System::Console::ResetTextColor(std::cout);

		std::cout << "  (";
		if (duration < 10000)
			std::cout << duration << "ms)";
		else
			std::cout << (duration / 1000) << "s)";
		std::cout << "\n\n";
	}


	bool runUnittsts(nyrun_cf_t& runcf, const std::vector<String>& optToRun, const char** filelist, uint32_t filecount)
	{
		if (optToRun.empty())
		{
			std::cerr << "error: no unit test name\n";
			return false;
		}

		uint32_t successCount = 0;
		uint32_t failCount = 0;
		int64_t starttime = DateTime::NowMilliSeconds();
		runcf.build.ignore_atoms = nytrue;
		runcf.build.userdata = nullptr;
		runcf.build.on_unittest = nullptr;

		for (auto& testname: optToRun)
		{
			bool localsuccess = runtest<true>(runcf, testname, filelist, filecount);
			++(localsuccess ? successCount : failCount);
		}

		int64_t duration = DateTime::NowMilliSeconds() - starttime;
		printStatstics(optToRun, duration, successCount, failCount);
		return (failCount == 0 and successCount != 0);
	}


	auto canonicalizeAllFilenames(std::vector<String>& remainingArgs)
	{
		auto filelist = std::make_unique<const char*[]>(remainingArgs.size());
		String filename;
		size_t i = 0;
		for (auto& arg: remainingArgs)
		{
			IO::Canonicalize(filename, arg);
			arg = filename;
			filelist[i++] = arg.c_str();
		}
		return filelist;
	}


	void shuffleUnittests(std::vector<String>& unittests)
	{
		std::cout << "shuffling the tests...\n";
		auto seed = std::chrono::system_clock::now().time_since_epoch().count();
		auto useed = static_cast<uint32_t>(seed);
		std::shuffle(unittests.begin(), unittests.end(), std::default_random_engine(useed));
	}


} // anonymous




int main(int argc, char** argv)
{
	std::vector<String> optToRun;
	std::vector<String> remainingArgs;
	bool optListAll = false;
	bool optWithNSLTests = false;
	bool optShuffle = false;
	uint32_t optRepeat = 1;

	// The command line options parser
	{
		GetOpt::Parser options;
		options.addFlag(optListAll, 'l', "list", "List all unit tests");
		options.addFlag(optToRun, 'r', "run", "Run a specific test");
		options.add(jobCount, 'j', "job", "Specifies the number of jobs (commands) to run simultaneously");
		options.addFlag(optShuffle, ' ', "shuffle", "Randomly rearrange the tests");
		options.add(optRepeat, ' ', "repeat", "Repeat the execution of the tests N times");
		options.addFlag(optWithNSLTests, ' ', "with-nsl", "Import NSL unittests");
		options.remainingArguments(remainingArgs);

		options.addParagraph("\nHelp");
		bool optBugreport = false;
		options.addFlag(optBugreport, ' ', "bugreport", "Display some useful information to report a bug");
		bool optVersion = false;
		options.addFlag(optVersion, 'v', "version", "Print the version");

		if (not options(argc, argv))
		{
			if (options.errors())
			{
				std::cout << "Abort due to error\n";
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}

		if (optVersion)
			return printVersion();
		if (optBugreport)
			return printBugReportInfo();
	}

	auto filecount = static_cast<uint32_t>(remainingArgs.size());
	if (filecount == 0)
		return printNoInputScript(argv[0]);
	auto filelist = canonicalizeAllFilenames(remainingArgs);

	nyrun_cf_t runcf;
	nyrun_cf_init(&runcf);
	if (optWithNSLTests)
		runcf.project.with_nsl_unittests = nytrue;

	int exitcode = EXIT_SUCCESS;
	hasColorsOut = System::Console::IsStdoutTTY();
	hasColorsErr = System::Console::IsStderrTTY();

	if (optListAll)
	{
		exitcode = listAllUnittests(runcf, filelist.get(), filecount);
	}
	else
	{
		if (hasColorsOut)
			System::Console::SetTextColor(std::cout, System::Console::bold);
		std::cout << "nanyc C++/boostrap unittest " << nylib_version();
		if (hasColorsOut)
			System::Console::ResetTextColor(std::cout);
		std::cout << '\n';
		nylib_print_info_for_bugreport();

		std::cout << ">\n";
		for (uint32_t i = 0; i != filecount; ++i)
			std::cout << "> from '" << filelist[i] << "'\n";
		std::cout << '\n';

		if (0 == jobCount or jobCount > 128)
			jobCount = System::CPU::Count();

		// no unittest provided from the command - default: all
		if (optToRun.empty())
			fetchUnittestList(runcf, optToRun, filelist.get(), filecount);

		for (uint32_t r = 0; r != optRepeat; ++r)
		{
			if (optShuffle)
				shuffleUnittests(optToRun);

			if (optShuffle or optRepeat > 1)
			{
				std::cout << "running " << optToRun.size() << " tests...";
				if (optRepeat > 1)
					std::cout << " (pass " << (r + 1) << '/' << optRepeat << ')';
				std::cout << '\n';
			}
			std::cout << '\n';

			bool success = runUnittsts(runcf, optToRun, filelist.get(), filecount);
			if (not success)
				exitcode = EXIT_FAILURE;
		}
	}
	return exitcode;
}
