#include <nanyc/library.h>
#include <nanyc/vm.h>
#include <yuni/core/getopt.h>
#include <yuni/core/process/program.h>
#include <yuni/core/string.h>
#include <yuni/core/system/console/console.h>
#include <yuni/core/system/cpu.h>
#include <yuni/datetime/timestamp.h>
#include <yuni/io/filename-manipulation.h>
#include <yuni/job/queue/service.h>
#include <yuni/thread/utility.h>
#include <yuni/yuni.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <random>
#include "libnanyc.h"


namespace ny {
namespace unittests {
namespace {

constexpr const char runningText[] = "running ";

struct Entry final {
	yuni::String module;
	yuni::String name;
};

struct Result final {
	Entry entry;
	bool success;
	int64_t duration_ms;
};

struct App final {
	App();
	App(const App&) = delete;
	App(App&&) = delete;
	~App();

	void importFilenames(const std::vector<AnyString>&);
	void fetch();
	void run(const Entry&);
	int run();
	bool statstics(int64_t duration);
	void setcolor(yuni::System::Console::Color) const;
	void resetcolor() const;
	bool inExecutorMode() const;

	struct final {
		uint32_t total = 0;
		uint32_t passing = 0;
		uint32_t failed = 0;
	}
	stats;
	nycompile_opts_t opts;
	bool interactive = true;
	bool colors = true;
	bool verbose = false;
	bool withnsl = false;
	uint32_t loops = 1;
	bool shuffle = false;
	uint32_t timeout_s = 30;
	std::vector<Entry> unittests;
	std::vector<yuni::String> filenames;
	std::vector<Result> results;
	uint32_t jobs = 0;
	yuni::Mutex mutex;

	Entry execinfo;
	AnyString argv0;

private:
	void startEntry(const Entry&);
	void endEntry(const Entry&, bool, int64_t);
	bool execute(const Entry& entry);
	bool writeProgress();

	yuni::String runningMsg;
	const Entry* latestRunningUnittest = nullptr;
	nysource_opts_t srcoptsEmpty;
};

App::App() {
	memset(&opts, 0x0, sizeof(opts));
	opts.userdata = this;
	memset(&srcoptsEmpty, 0x0, sizeof(srcoptsEmpty));
	srcoptsEmpty.content.c_str = " ";
	srcoptsEmpty.content.len = 1;
}

App::~App() {
	free(opts.sources.items);
}

void App::setcolor(yuni::System::Console::Color c) const {
	if (colors)
		yuni::System::Console::SetTextColor(std::cout, c);
}

void App::resetcolor() const {
	if (colors)
		yuni::System::Console::ResetTextColor(std::cout);
}

bool App::inExecutorMode() const {
	return not execinfo.name.empty();
}

auto now() {
	return yuni::DateTime::NowMilliSeconds();
}

bool operator < (const Entry& a, const Entry& b) {
	return std::tie(a.module, a.name) < std::tie(b.module, b.name);
}

const char* plurals(size_t count, const char* single, const char* many) {
	return (count <= 1) ? single : many;
}

uint32_t numberOfJobs(uint32_t jobs) {
	if (jobs == 0) {
		jobs = yuni::System::CPU::Count();
	}
	else if (jobs > 256) // arbitrary
		jobs = 256;
	return jobs;
}

void App::importFilenames(const std::vector<AnyString>& list) {
	uint32_t count = static_cast<uint32_t>(list.size());
	if (unlikely(count == 0))
		return;
	filenames.resize(count);
	std::transform(std::begin(list), std::end(list), std::begin(filenames), [](auto& item) -> yuni::String {
		return std::move(yuni::IO::Canonicalize(item));
	});
	opts.sources.count = count;
	opts.sources.items = (nysource_opts_t*) calloc(count, sizeof(nysource_opts_t));
	if (unlikely(!opts.sources.items))
		throw std::bad_alloc();
	for (uint32_t i = 0; i != count; ++i) {
		opts.sources.items[i].filename.len = filenames[i].size();
		opts.sources.items[i].filename.c_str = filenames[i].c_str();
	}
}

void App::fetch() {
	unittests.reserve(512); // arbitrary
	opts.on_unittest = [](void* userdata, const char* mod, uint32_t mlen, const char* name, uint32_t nlen) {
		auto& self = *reinterpret_cast<App*>(userdata);
		self.unittests.emplace_back();
		auto& entry = self.unittests.back();
		entry.module.assign(mod, mlen);
		entry.name.assign(name, nlen);
	};
	if (opts.sources.count == 0) {
		opts.sources.count = 1;
		opts.sources.items = &srcoptsEmpty;
	}
	opts.entrypoint.len = 0;
	std::cout << "searching for unittests in all source files...\n";
	auto start = now();
	auto* program = nyprogram_compile(&opts);
	if (unlikely(program))
		throw "expecting a null object when gathering the list of unittests";
	if (opts.sources.items == &srcoptsEmpty) {
		opts.sources.count = 0;
		opts.sources.items = nullptr;
	}
	opts.on_unittest = nullptr;
	std::sort(std::begin(unittests), std::end(unittests));
	auto duration = now() - start;
	std::cout << unittests.size() << ' ' << plurals(unittests.size(), "test", "tests");
	std::cout << " found (in " << duration << "ms)\n";
}

void App::startEntry(const Entry& entry) {
	if (interactive) {
		yuni::MutexLocker locker(mutex);
		latestRunningUnittest = &entry;
	}
}

void App::endEntry(const Entry& entry, bool success, int64_t duration) {
	Result result;
	result.entry = entry;
	result.success = success;
	result.duration_ms = duration;
	yuni::MutexLocker locker(mutex);
	results.emplace_back(std::move(result));
}

bool App::writeProgress() {
	size_t achieved = 0;
	const Entry* latestEntry = nullptr;
	{
		yuni::MutexLocker locker(mutex);
		achieved = results.size();
		latestEntry = latestRunningUnittest;
	}
	if (unlikely(!latestEntry))
		return true;
	auto& entry = *latestEntry;
	uint32_t progress = static_cast<uint32_t>((100. / stats.total) * static_cast<uint32_t>(achieved));
	if (progress > 99)
		progress = 99;
	if (progress < 10)
		std::cout << "   ";
	else
		std::cout << "  ";
	std::cout << static_cast<uint32_t>(progress) << "% - ";
	setcolor(yuni::System::Console::bold);
	std::cout << runningText;
	resetcolor();
	auto previousLength = runningMsg.size();
	runningMsg.clear();
	runningMsg << entry.module << '/' << entry.name << "... ";
	if (runningMsg.size() < previousLength)
		runningMsg.resize(previousLength, " ");
	std::cout << runningMsg << '\r' << std::flush;
	return true;
}

bool App::statstics(int64_t duration) {
	if (interactive) {
		runningMsg.resize(runningMsg.size() + AnyString(runningText).size() + 8 /*%*/);
		runningMsg.fill(' ');
		std::cout << runningMsg << '\r';
	}
	for (auto& result: results) {
		++(result.success ? stats.passing : stats.failed);
		if (result.success) {
			setcolor(yuni::System::Console::green);
			#ifndef YUNI_OS_WINDOWS
			std::cout << "    \u2713  ";
			#else
			std::cout << "   OK  ";
			#endif
			resetcolor();
		}
		else {
			setcolor(yuni::System::Console::red);
			std::cout << "  FAIL ";
			resetcolor();
		}
		std::cout << result.entry.module << '/' << result.entry.name;
		setcolor(yuni::System::Console::lightblue);
		std::cout << "  (" << result.duration_ms << "ms)";
		resetcolor();
		std::cout << '\n';
	}
	std::cout << "\n       " << stats.total << ' ' << plurals(stats.total, "test", "tests");
	if (stats.passing != 0) {
		std::cout << ", ";
		setcolor(yuni::System::Console::green);
		std::cout << stats.passing << " passing";
		resetcolor();
	}
	if (stats.failed) {
		std::cout << ", ";
		setcolor(yuni::System::Console::red);
		std::cout << stats.failed << " failed";
		resetcolor();
	}
	std::cout << "  (";
	if (duration < 10000)
		std::cout << duration << "ms)";
	else
		std::cout << (duration / 1000) << "s)";
	std::cout << "\n\n";
	return stats.failed == 0 and stats.total != 0;
}

void report(void*, const nyreport_t* report) {
	nyreport_print_stdout(report);
}

bool App::execute(const Entry& entry) {
	yuni::ShortString128 name;
	name << "^unittest^module:" << entry.name;
	opts.entrypoint.c_str = name.c_str();
	opts.entrypoint.len = name.size();
	opts.on_report = &report;
	if (opts.sources.count == 0) {
		opts.sources.count = 1;
		opts.sources.items = &srcoptsEmpty;
	}
	auto* program = nyprogram_compile(&opts);
	if (opts.sources.items == &srcoptsEmpty) {
		opts.sources.count = 0;
		opts.sources.items = nullptr;
	}
	if (program != nullptr) {
		nyvm_opts_t vmopts;
		nyvm_opts_init_defaults(&vmopts);
		bool success = (nytrue == nyvm_run_entrypoint(&vmopts, program));
		nyprogram_free(program);
		return success;
	}
	return false;
}

void App::run(const Entry& entry) {
	startEntry(entry);
	yuni::Process::Program program;
	program.durationPrecision(yuni::Process::Program::dpMilliseconds);
	program.program(argv0);
	program.argumentAdd("--executor-module");
	program.argumentAdd(entry.module);
	program.argumentAdd("--executor-name");
	program.argumentAdd(entry.name);
	if (withnsl)
		program.argumentAdd("--nsl");
	for (auto& filename: filenames)
		program.argumentAdd(filename);
	auto start = now();
	bool success = program.execute(timeout_s);
	success = success and (program.wait() == 0);
	auto duration = now() - start;
	endEntry(entry, success, duration);
}

void shuffleDeck(std::vector<Entry>& unittests) {
	std::cout << "shuffling the tests...\n" << std::flush;
	auto seed = now();
	auto useed = static_cast<uint32_t>(seed);
	std::shuffle(unittests.begin(), unittests.end(), std::default_random_engine(useed));
}

int App::run() {
	bool success;
	if (not inExecutorMode()) {
		if (verbose or not interactive) {
			std::cout << '\n';
			setcolor(yuni::System::Console::bold);
			std::cout << "running all tests (" << jobs << " concurrent " << plurals(jobs, "job", "jobs") << ")...";
			resetcolor();
			std::cout << '\n';
		}
		stats.total = static_cast<uint32_t>(loops * unittests.size());
		results.reserve(stats.total);
		std::cout << '\n';
		yuni::Job::QueueService queueservice;
		queueservice.maximumThreadCount(jobs);
		queueservice.minimumThreadCount(jobs);
		for (uint32_t l = 0; l != loops; ++l) {
			if (unlikely(shuffle))
				shuffleDeck(unittests);
			for (auto& entry: unittests)
				yuni::async(queueservice, [=] { run(entry); });
		}
		auto start = now();
		auto duration = start;
		{
			auto progress = yuni::every(150 /*ms*/, [&] { return writeProgress(); });
			queueservice.start();
			queueservice.wait(yuni::qseIdle);
			duration = now() - start;
		}
		success = statstics(duration);
	}
	else {
		success = execute(execinfo);
	}
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

int printVersion() {
	std::cout << libnanyc_version_to_cstr() << '\n';
	return EXIT_SUCCESS;
}

int printBugreport() {
	uint32_t length;
	auto* text = libnanyc_get_bugreportdetails(&length);
	if (text) {
		std::cout.write(text, length);
		free(text);
	}
	std::cout << '\n';
	return EXIT_SUCCESS;
}

void prepare(App& app, int argc, char** argv) {
	bool version = false;
	bool bugreport = false;
	bool nocolors = false;
	bool nointeractive = false;
	std::vector<AnyString> filenames;
	yuni::GetOpt::Parser options;
	options.add(filenames, 'i', "", "Input nanyc source files");
	options.addFlag(app.withnsl, ' ', "nsl", "Import NSL unittests");
	options.add(app.timeout_s, 't', "timeout", "Timeout for executing an unittest (seconds)");
	options.add(app.jobs, 'j', "jobs", "Number of concurrent jobs (default: auto)");
	options.add(app.execinfo.module, ' ', "executor-module", "Executor mode, module name (internal use)", false);
	options.add(app.execinfo.name, ' ', "executor-name", "Executor mode, unittest (internal use)", false);
	options.add(app.loops, 'n', "loops", "Number of loops (default: 1)");
	options.addFlag(app.shuffle, 's', "shuffle", "Randomly rearrange the unittests");
	options.addParagraph("\nDisplay");
	options.addFlag(nocolors, ' ', "no-colors", "Disable color output");
	options.addFlag(nointeractive, ' ', "no-progress", "Disable progression reporting");
	options.addParagraph("\nHelp");
	options.addFlag(app.verbose, 'v', "verbose", "More stuff on the screen");
	options.addFlag(bugreport, 'b', "bugreport", "Display some useful information to report a bug");
	options.addFlag(version, ' ', "version", "Print the version");
	options.remainingArguments(filenames);
	if (not options(argc, argv)) {
		if (options.errors())
			throw std::runtime_error("Abort due to error");
		throw EXIT_SUCCESS;
	}
	if (unlikely(version))
		throw printVersion();
	if (unlikely(bugreport))
		throw printBugreport();
	if (unlikely(app.verbose))
		printBugreport();
	app.importFilenames(filenames);
	app.opts.with_nsl_unittests = app.withnsl ? nytrue : nyfalse;
	if (not app.inExecutorMode()) {
		if (unlikely(app.loops > 100))
			throw "number of loops greater than hard-limit '100'";
		if (unlikely(app.timeout_s == 0))
			throw "invalid null timeout (-t,--timeout)";
		bool istty = yuni::System::Console::IsStdoutTTY();
		app.interactive = not nointeractive and istty;
		app.colors = (not nocolors) and istty;
		app.argv0 = argv[0];
		app.jobs = numberOfJobs(app.jobs);
		app.fetch();
	}
}

} // namespace
} // namespace unittests
} // namespace ny

int main(int argc, char** argv) {
	try {
		ny::unittests::App app;
		ny::unittests::prepare(app, argc, argv);
		return app.run();
	}
	catch (const char* e) {
		std::cerr << "error: " << e << '\n';
	}
	catch (const std::exception& e) {
		std::cerr << "exception: " << e.what() << '\n';
	}
	catch (int e) {
		return e;
	}
	return EXIT_FAILURE;;
}
