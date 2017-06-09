#include <nanyc/library.h>
#include <nanyc/program.h>
#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <yuni/core/string.h>
#include <yuni/io/filename-manipulation.h>
#include <yuni/datetime/timestamp.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include "libnanyc.h"


namespace ny {
namespace unittests {
namespace {

struct Entry final {
	yuni::String module;
	yuni::String name;
};

struct App final {
	App();
	App(App&&) = default;
	~App();

	void importFilenames(const std::vector<AnyString>&);
	void fetch(bool nsl);
	void run(const Entry&);
	int run();

	nycompile_opts_t opts;
	bool interactive = true;
	std::vector<Entry> unittests;
	std::vector<yuni::String> filenames;

private:
	void startEntry(const Entry&);
	void endEntry(const Entry&, bool, int64_t);
};

App::App() {
	memset(&opts, 0x0, sizeof(opts));
	opts.userdata = this;;
}

App::~App() {
	free(opts.sources.items);
}

auto now() {
	return yuni::DateTime::NowMilliSeconds();
}

bool operator < (const Entry& a, const Entry& b) {
	return std::tie(a.module, a.name) < std::tie(b.module, b.name);
}

const char* plurals(auto count, const char* single, const char* many) {
	return (count <= 1) ? single : many;
}

void App::importFilenames(const std::vector<AnyString>& list) {
	uint32_t count = static_cast<uint32_t>(list.size());
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

void App::fetch(bool nsl) {
	unittests.reserve(512); // arbitrary
	opts.with_nsl_unittests = nsl ? nytrue : nyfalse;
	opts.on_unittest = [](void* userdata, const char* mod, uint32_t mlen, const char* name, uint32_t nlen) {
		auto& self = *reinterpret_cast<App*>(userdata);
		self.unittests.emplace_back();
		auto& entry = self.unittests.back();
		entry.module.assign(mod, mlen);
		entry.name.assign(name, nlen);
	};
	std::cout << "searching for unittests in all source files...\n";
	auto start = now();
	nyprogram_compile(&opts);
	opts.on_unittest = nullptr;
	std::sort(std::begin(unittests), std::end(unittests));
	auto duration = now() - start;
	std::cout << unittests.size() << ' ' << plurals(unittests.size(), "test", "tests");
	std::cout << " found (in " << duration << "ms)\n";
}

void App::startEntry(const Entry& entry) {
	if (interactive) {
		std::cout << "       running ";
		std::cout << entry.module << '/' << entry.name;
		std::cout << "... " << std::flush;
	}
}

void App::endEntry(const Entry& entry, bool success, int64_t duration) {
	if (interactive)
		std::cout << '\r'; // back to begining of the line
	if (success) {
		#ifndef YUNI_OS_WINDOWS
		std::cout << "    \u2713  ";
		#else
		std::cout << "   OK  ";
		#endif
	}
	else {
		std::cout << "  ERR  ";
	}
	std::cout << entry.module << '/' << entry.name;
	std::cout << "  (" << duration << "ms)    ";
	std::cout << '\n';
}

void App::run(const Entry& entry) {
	startEntry(entry);
	auto start = now();
	auto* program = nyprogram_compile(&opts);
	bool success = program != nullptr;
	if (program) {
		nyprogram_free(program);
	}
	auto duration = now() - start;
	endEntry(entry, success, duration);
}

int App::run() {
	std::cout << '\n';
	for (auto& entry: unittests)
		run(entry);
	return EXIT_SUCCESS;
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

App prepare(int argc, char** argv) {
	App app;
	bool version = false;
	bool bugreport = false;
	bool nsl = false;
	std::vector<AnyString> filenames;
	yuni::GetOpt::Parser options;
	options.addFlag(filenames, 'i', "", "Input nanyc source files");
	options.addFlag(nsl, ' ', "nsl", "Import NSL unittests");
	options.addParagraph("\nHelp");
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
	app.importFilenames(filenames);
	app.fetch(nsl);
	return app;
}

} // namespace
} // namespace unittests
} // namespace ny

int main(int argc, char** argv) {
	try {
		auto app = ny::unittests::prepare(argc, argv);
		return app.run();
	}
	catch (const std::exception& e) {
		std::cerr << "exception: " << e.what() << '\n';
	}
	catch (int e) {
		return e;
	}
	return EXIT_FAILURE;;
}
