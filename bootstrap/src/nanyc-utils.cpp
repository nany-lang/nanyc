#include "nanyc-utils.h"
#include <nanyc/library.h>
#include <yuni/string.h>
#include <iostream>


namespace ny {
namespace print {

int noInputScript(const char* const argv0) {
	std::cerr << argv0 << ": no input script file\n";
	return EXIT_FAILURE;
}

int usage(const char* const argv0) {
	std::cout << "Usage: " << argv0 << " [options] file...\n";
	std::cout << "Options:\n";
	std::cout << "  --bugreport       Display some useful information to report a bug\n";
	std::cout << "                    (https://github.com/nany-lang/nany/issues/new)\n";
	std::cout << "  --help, -h        Display this information\n";
	std::cout << "  --version, -v     Print the version\n\n";
	return EXIT_SUCCESS;
}

int bugReportInfo() {
	uint32_t len = 0;
	char* report = libnanyc_get_bugreportdetails(&len);
	if (report != nullptr) {
		std::cout.write(report, len);
		free(report);
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int version() {
	std::cout << libnanyc_version_to_cstr() << '\n';
	return EXIT_SUCCESS;
}

int unknownOption(const char* const argv0, const char* const name) {
	std::cerr << argv0 << ": unknown option '" << name << "'\n";
	return EXIT_FAILURE;
}

void fileAccessError(const nyproject_t*, nybuild_t*, const char* file, uint32_t length) {
	std::cerr << "error: failed to access to '";
	std::cerr << AnyString{file, length} << "'\n";
}

} // namespace print
} // namespace ny
