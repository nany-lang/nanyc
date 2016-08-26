#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <nany/nany.h>
#include <yuni/datetime/timestamp.h>
#include <yuni/core/system/cpu.h>
#include <yuni/core/system/console/console.h>
#include <set>
#include <iostream>
#include <cassert>

using namespace Yuni;

static uint32_t jobCount = 0;
static bool hasColorsOut = false;
static bool hasColorsErr = false;


static int printBugReportInfo()
{
	nylib_print_info_for_bugreport();
	return EXIT_SUCCESS;
}


static int printVersion()
{
	std::cout << nylib_version() << '\n';
	return EXIT_SUCCESS;
}


static int printNoInputScript(const char* argv0)
{
	std::cerr << argv0 << ": no input script file\n";
	return EXIT_FAILURE;
}


static void fetchUnittestList(nyrun_cf_t& runcf, String::Vector& torun, const char** filelist, uint32_t count)
{
	runcf.build.entrypoint.size  = 0; // disable any compilation by default
	runcf.build.entrypoint.c_str = nullptr;
	runcf.build.ignore_atoms = nytrue;
	runcf.build.userdata = &torun;
	runcf.build.on_unittest = [](void* userdata, const char* mod, uint32_t modlen, const char* name, uint32_t nlen)
	{
		AnyString module{mod, modlen};
		AnyString testname{name, nlen};
		((String::Vector*) userdata)->emplace_back();
		auto& element = ((String::Vector*) userdata)->back();
		element << module << ':' << testname;
	};
	nyrun_filelist(&runcf, filelist, count, 0, nullptr);
}



static int listAllUnittests(nyrun_cf_t& runcf, const char** filelist, uint32_t count)
{
	std::set<std::pair<String, String>> alltests;

	runcf.build.entrypoint.size  = 0; // disable any compilation by default
	runcf.build.entrypoint.c_str = nullptr;
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
static bool runtest(nyrun_cf_t& runcf, const String& testname, const char** filelist, uint32_t count)
{
	ShortString256 entry;
	entry << "^unittest^" << testname;
	entry.replace("<nomodule>", "module");
	runcf.build.entrypoint.size  = entry.size();
	runcf.build.entrypoint.c_str = entry.c_str();
	runcf.build.ignore_atoms = nytrue;
	runcf.build.userdata = nullptr;
	runcf.build.on_unittest = nullptr;

	bool interactive = (hasColorsOut and jobCount == 1);

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

	if (Fancy)
	{
		if (interactive)
			std::cout << '\r';

		if (success)
		{
			if (hasColorsOut)
				System::Console::SetTextColor(std::cout, System::Console::green);
			std::cout << "    \u2713  ";
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
		std::cout << std::endl;
	}
	return success;
}


static void printStatstics(const String::Vector& optToRun, int64_t duration, uint32_t successCount, uint32_t failCount)
{
	switch (optToRun.size())
	{
		case 0:  std::cout << "\n       0 test, "; break;
		case 1:  std::cout << "\n       1 test"; break;
		default: std::cout << "\n       " << optToRun.size() << " test, "; break;
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


static bool runUnittsts(nyrun_cf_t& runcf, const String::Vector& optToRun, const char** filelist, uint32_t filecount)
{
	if (optToRun.empty())
	{
		std::cerr << "error: no unit test name\n";
		return false;
	}

	uint32_t successCount = 0;
	uint32_t failCount = 0;
	int64_t starttime = DateTime::NowMilliSeconds();

	for (auto& testname: optToRun)
	{
		bool localsuccess = runtest<true>(runcf, testname, filelist, filecount);
		++(localsuccess ? successCount : failCount);
	}

	int64_t duration = DateTime::NowMilliSeconds() - starttime;
	printStatstics(optToRun, duration, successCount, failCount);
	return (failCount == 0 and successCount != 0);
}




int main(int argc, char** argv)
{
	// options
	String::Vector optToRun;
	String::Vector remainingArgs;
	bool optListAll = false;

	// The command line options parser
	{
		GetOpt::Parser options;
		options.addFlag(optListAll, 'l', "list", "List all unit tests");
		options.addFlag(optToRun, 'r', "run", "Run a specific test");
		options.add(jobCount, 'j', "job", "Specifies the number of jobs (commands) to run simultaneously");
		options.remainingArguments(remainingArgs);

		// Help
		options.addParagraph("\nHelp");
		bool optBugreport = false;
		options.addFlag(optBugreport, ' ', "bugreport", "Display some useful information to report a bug");
		bool optVersion = false;
		options.addFlag(optVersion, 'v', "version", "Print the version");

		if (not options(argc, argv))
		{
			if (options.errors())
			{
				std::cout << "Abort due to error" << std::endl;
				return 1;
			}
			return 0;
		}

		if (optVersion)
			return printVersion();
		if (optBugreport)
			return printBugReportInfo();
	}

	uint32_t filecount = (uint32_t) remainingArgs.size();
	if (filecount == 0)
		return printNoInputScript(argv[0]);

	auto** filelist = (const char**) malloc(sizeof(char*) * filecount);
	for (uint32_t i = 0; i != filecount; ++i)
		filelist[i] = remainingArgs[i].c_str();

	nyrun_cf_t runcf;
	nyrun_cf_init(&runcf);

	int exitcode = EXIT_SUCCESS;
	hasColorsOut = System::Console::IsStdoutTTY();
	hasColorsErr = System::Console::IsStderrTTY();

	if (optListAll)
	{
		exitcode = listAllUnittests(runcf, filelist, filecount);
	}
	else
	{
		if (0 == jobCount)
			System::CPU::Count();

		// no unittest provided from the command - default: all
		if (optToRun.empty())
			fetchUnittestList(runcf, optToRun, filelist, filecount);

		bool success = runUnittsts(runcf, optToRun, filelist, filecount);
		exitcode = (success) ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	free(filelist);
	return exitcode;
}
