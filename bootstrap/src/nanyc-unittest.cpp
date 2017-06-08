#include <nanyc/library.h>
#include <nanyc/program.h>
#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <iostream>
#include "libnanyc.h"


namespace ny {
namespace unittests {
namespace {

struct App final {
	App();

	nycompile_opts_t opts;
};

App::App() {
	memset(&opts, 0x0, sizeof(opts));
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
	yuni::GetOpt::Parser options;
	options.addParagraph("\nHelp");
	options.addFlag(bugreport, 'b', "bugreport", "Display some useful information to report a bug");
	options.addFlag(version, ' ', "version", "Print the version");
	if (not options(argc, argv)) {
		if (options.errors())
			throw std::runtime_error("Abort due to error");
		throw EXIT_SUCCESS;
	}
	if (unlikely(version))
		throw printVersion();
	if (unlikely(bugreport))
		throw printBugreport();
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
