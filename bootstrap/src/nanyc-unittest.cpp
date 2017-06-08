#include <nanyc/library.h>
#include <nanyc/program.h>
#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <yuni/core/string.h>
#include <yuni/io/filename-manipulation.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include "libnanyc.h"


namespace ny {
namespace unittests {
namespace {

struct App final {
	App();
	App(App&&) = default;
	~App();

	void importFilenames(const std::vector<AnyString>&);

	nycompile_opts_t opts;
	std::vector<yuni::String> filenames;
};

App::App() {
	memset(&opts, 0x0, sizeof(opts));
}

App::~App() {
	free(opts.sources.items);
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
	std::vector<AnyString> filenames;
	yuni::GetOpt::Parser options;
	options.addFlag(filenames, 'i', "", "Input nanyc source files");
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
	return app;
}

} // namespace
} // namespace unittests
} // namespace ny

int main(int argc, char** argv) {
	try {
		auto app = ny::unittests::prepare(argc, argv);
		return EXIT_SUCCESS;
	}
	catch (const std::exception& e) {
		std::cerr << "exception: " << e.what() << '\n';
	}
	catch (int e) {
		return e;
	}
	return EXIT_FAILURE;;
}
