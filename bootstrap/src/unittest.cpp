#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <nany/nany.h>
#include <yuni/datetime/timestamp.h>
#include <yuni/core/system/cpu.h>
#include <yuni/core/system/console/console.h>
#include <yuni/io/file.h>
#include <set>
#include <memory>
#include <iostream>
#include <cassert>
#include <random>
#include <chrono>

using namespace Yuni;


namespace {


struct Settings final {
	std::vector<String> unittests;
	std::vector<String> remainingArgs;
	bool listAll = false;
	bool nslTests = false;
	bool shuffle = false;
	uint32_t jobs = 0;
	uint32_t repeat = 1;
	struct {
		bool out = false;
		bool err = false;
	}
	colors;
};

bool printBugReportInfo() {
	nylib_print_info_for_bugreport();
	return false;
}

bool printVersion() {
	std::cout << nylib_version() << '\n';
	return false;
}

void shuffleUnittests(std::vector<String>& unittests) {
	std::cout << "shuffling the tests...\n";
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto useed = static_cast<uint32_t>(seed);
	std::shuffle(unittests.begin(), unittests.end(), std::default_random_engine(useed));
}

bool parseCommandLine(Settings& settings, int argc, char** argv) {
	GetOpt::Parser options;
	options.addFlag(settings.listAll, 'l', "list", "List all unit tests");
	options.addFlag(settings.unittests, 'r', "run", "Run a specific test");
	options.add(settings.jobs, 'j', "job", "Specifies the number of jobs (commands) to run simultaneously");
	options.addFlag(settings.shuffle, ' ', "shuffle", "Randomly rearrange the tests");
	options.add(settings.repeat, ' ', "repeat", "Repeat the execution of the tests N times");
	options.addFlag(settings.nslTests, ' ', "with-nsl", "Import NSL unittests");
	options.remainingArguments(settings.remainingArgs);
	options.addParagraph("\nHelp");
	bool bugreport = false;
	options.addFlag(bugreport, ' ', "bugreport", "Display some useful information to report a bug");
	bool version = false;
	options.addFlag(version, 'v', "version", "Print the version");
	if (not options(argc, argv)) {
		if (options.errors())
			throw std::runtime_error("Abort due to error");
		return false;
	}
	if (version)
		return printVersion();
	if (bugreport)
		return printBugReportInfo();
	if (settings.remainingArgs.empty())
		throw std::runtime_error(std::string{argv[0]} += ": no input script file");
	return true;
}

void fetchUnittestList(nyrun_cf_t& runcf, std::vector<String>& torun, const char** filelist, uint32_t count) {
	std::cout << "searching for unittests in all source files...\n" << std::flush;
	runcf.build.entrypoint.size = 0; // disable any compilation by default
	runcf.build.entrypoint.c_str = nullptr;
	runcf.program.entrypoint.size = 0;
	runcf.program.entrypoint.c_str = nullptr;
	runcf.build.ignore_atoms = nytrue;
	runcf.build.userdata = &torun;
	runcf.build.on_unittest = [](void* userdata, const char* mod, uint32_t modlen, const char* name, uint32_t nlen) {
		AnyString module{mod, modlen};
		AnyString testname{name, nlen};
		((std::vector<String>*) userdata)->emplace_back();
		auto& element = ((std::vector<String>*) userdata)->back();
		element << module << ':' << testname;
	};
	int64_t starttime = DateTime::NowMilliSeconds();
	nyrun_filelist(&runcf, filelist, count, 0, nullptr);
	int64_t duration = DateTime::NowMilliSeconds() - starttime;
	switch (torun.size()) {
		case 0: std::cout << "0 test found"; break;
		case 1: std::cout << "1 test found"; break;
		default: std::cout << torun.size() << " tests found"; break;
	}
	std::cout << " (" << duration << "ms)\n";
}

int listAllUnittests(nyrun_cf_t& runcf, const Settings& settings, const char** filelist, uint32_t count) {
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
			if (settings.colors.out)
				System::Console::SetTextColor(std::cout, System::Console::bold);
			std::cout << '.' << lastmodule << ":\n";
			if (settings.colors.out)
				System::Console::ResetTextColor(std::cout);
		}
		std::cout << "       ";
		if (not module.empty())
			std::cout << module << ':';
		std::cout << pair.second << '\n';
	}
	std::cout << "\n       ";
	if (settings.colors.out)
		System::Console::SetTextColor(std::cout, System::Console::lightblue);
	switch (moduleCount) {
		case 0:  break;
		case 1:  std::cout << "1 module, "; break;
		default: std::cout << moduleCount << " modules, "; break;
	}
	switch (alltests.size()) {
		case 0:  std::cout << "0 test found"; break;
		case 1:  std::cout << "1 test found"; break;
		default: std::cout << alltests.size() << " tests found"; break;
	}
	if (settings.colors.out)
		System::Console::ResetTextColor(std::cout);
	std::cout << "  (" << duration << "ms)\n\n";
	return EXIT_SUCCESS;
}

template<bool Fancy>
bool runtest(nyrun_cf_t& originalRuncf, const Settings& settings, const String& testname, const char** filelist, uint32_t count) {
	ShortString256 entry;
	entry << "^unittest^" << testname;
	entry.replace("<nomodule>", "module");
	bool interactive = (settings.colors.out and settings.jobs == 1);
	auto runcf = originalRuncf;
	runcf.build.entrypoint.size  = entry.size();
	runcf.build.entrypoint.c_str = entry.c_str();
	runcf.program.entrypoint = runcf.build.entrypoint;
	runcf.build.ignore_atoms = nyfalse;
	struct Console {
		Clob cout, cerr;
	} console;
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
	if (interactive and Fancy) {
		if (settings.colors.out)
			System::Console::SetTextColor(std::cout, System::Console::bold);
		std::cout << "       running ";
		if (settings.colors.out)
			System::Console::ResetTextColor(std::cout);
		std::cout << testname << "... " << std::flush;
	}
	int64_t starttime = DateTime::NowMilliSeconds();
	bool success = !nyrun_filelist(&runcf, filelist, count, 0, nullptr);
	int64_t duration = DateTime::NowMilliSeconds() - starttime;
	success &= console.cerr.empty();
	if (Fancy) {
		if (interactive)
			std::cout << '\r';
		if (success) {
			if (settings.colors.out)
				System::Console::SetTextColor(std::cout, System::Console::green);
			#ifndef YUNI_OS_WINDOWS
			std::cout << "    \u2713  ";
			#else
			std::cout << "   OK  ";
			#endif
		}
		else {
			if (settings.colors.out)
				System::Console::SetTextColor(std::cout, System::Console::red);
			std::cout << "  ERR  ";
		}
		if (settings.colors.out)
			System::Console::ResetTextColor(std::cout);
		std::cout << testname;
		if (duration < 1200 or not settings.colors.out) {
			std::cout << "  (" << duration << "ms)     ";
		}
		else {
			System::Console::SetTextColor(std::cout, System::Console::purple);
			std::cout << "  (" << duration << "ms)     ";
			System::Console::ResetTextColor(std::cout);
		}
		std::cout << '\n';
		if (not console.cerr.empty()) {
			console.cerr.trimRight();
			console.cerr.replace("\n", "\n       | ");
			std::cout << "       | " << console.cerr << '\n';
		}
		std::cout << std::flush;
	}
	return success;
}

void printStatstics(const Settings& settings, int64_t duration, uint32_t successCount, uint32_t failCount) {
	switch (settings.unittests.size()) {
		case 0:  std::cout << "\n       0 test, "; break;
		case 1:  std::cout << "\n       1 test"; break;
		default: std::cout << "\n       " << settings.unittests.size() << " tests, "; break;
	}
	if (successCount and settings.colors.out)
		System::Console::SetTextColor(std::cout, System::Console::green);
	switch (successCount) {
		case 0:  std::cout << "0 passing"; break;
		case 1:  std::cout << "1 passing"; break;
		default: std::cout << successCount << " passing"; break;
	}
	if (successCount and settings.colors.out)
		System::Console::ResetTextColor(std::cout);
	if (failCount)
		std::cout << ", ";

	if (failCount and settings.colors.out)
		System::Console::SetTextColor(std::cout, System::Console::red);
	switch (failCount) {
		case 0:  break;
		case 1:  std::cout << "1 failed"; break;
		default: std::cout << "" << failCount << " failed"; break;
	}
	if (failCount and settings.colors.out)
		System::Console::ResetTextColor(std::cout);
	std::cout << "  (";
	if (duration < 10000)
		std::cout << duration << "ms)";
	else
		std::cout << (duration / 1000) << "s)";
	std::cout << "\n\n";
}

bool runUnittests(nyrun_cf_t& runcf, const Settings& settings, const char** filelist, uint32_t filecount) {
	if (settings.unittests.empty())
		throw std::runtime_error("error: no unit test name");
	runcf.build.ignore_atoms = nytrue;
	runcf.build.userdata = nullptr;
	runcf.build.on_unittest = nullptr;
	uint32_t successCount = 0;
	uint32_t failCount = 0;
	int64_t starttime = DateTime::NowMilliSeconds();
	for (auto& testname: settings.unittests) {
		bool localsuccess = runtest<true>(runcf, settings, testname, filelist, filecount);
		++(localsuccess ? successCount : failCount);
	}
	int64_t duration = DateTime::NowMilliSeconds() - starttime;
	printStatstics(settings, duration, successCount, failCount);
	return (failCount == 0 and successCount != 0);
}

auto canonicalizeAllFilenames(std::vector<String>& remainingArgs) {
	auto filelist = std::make_unique<const char*[]>(remainingArgs.size());
	String filename;
	size_t i = 0;
	for (auto& arg: remainingArgs) {
		IO::Canonicalize(filename, arg);
		arg = filename;
		filelist[i++] = arg.c_str();
	}
	return filelist;
}

int runAllUnittests(nyrun_cf_t& runcf, Settings& settings, const char** filelist, uint32_t filecount) {
	if (settings.colors.out)
		System::Console::SetTextColor(std::cout, System::Console::bold);
	std::cout << "nanyc C++/boostrap unittest " << nylib_version();
	if (settings.colors.out)
		System::Console::ResetTextColor(std::cout);
	std::cout << '\n';
	nylib_print_info_for_bugreport();
	std::cout << ">\n";
	for (uint32_t i = 0; i != filecount; ++i)
		std::cout << "> from '" << filelist[i] << "'\n";
	std::cout << '\n';
	if (settings.jobs == 0 or settings.jobs > 128)
		settings.jobs = 1; // System::CPU::Count();
	// no unittest provided from the command - default: all
	if (settings.unittests.empty())
		fetchUnittestList(runcf, settings.unittests, filelist, filecount);

	int exitcode = EXIT_SUCCESS;
	for (uint32_t r = 0; r != settings.repeat; ++r) {
		if (settings.shuffle)
			shuffleUnittests(settings.unittests);

		if (settings.shuffle or settings.repeat > 1) {
			std::cout << "running " << settings.unittests.size() << " tests...";
			if (settings.repeat > 1) {
				if (settings.colors.out)
					System::Console::SetTextColor(std::cout, System::Console::bold);
				std::cout << " (pass " << (r + 1) << '/' << settings.repeat << ')';
				if (settings.colors.out)
					System::Console::ResetTextColor(std::cout);
			}
			std::cout << '\n';
		}
		std::cout << '\n';
		bool success = runUnittests(runcf, settings, filelist, filecount);
		if (not success)
			exitcode = EXIT_FAILURE;
	}
	return exitcode;
}


} // anonymous


int main(int argc, char** argv)
{
	try {
		Settings settings;
		if (not parseCommandLine(settings, argc, argv))
			return EXIT_SUCCESS;
		auto filecount = static_cast<uint32_t>(settings.remainingArgs.size());
		auto filelist = canonicalizeAllFilenames(settings.remainingArgs);
		settings.colors.out = System::Console::IsStdoutTTY();
		settings.colors.err = System::Console::IsStderrTTY();

		nyrun_cf_t runcf;
		nyrun_cf_init(&runcf);
		if (settings.nslTests)
			runcf.project.with_nsl_unittests = nytrue;
		return (settings.listAll)
			? listAllUnittests(runcf, settings, filelist.get(), filecount)
			: runAllUnittests(runcf, settings, filelist.get(), filecount);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
	return EXIT_FAILURE;
}
