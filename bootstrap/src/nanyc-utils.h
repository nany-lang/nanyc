#pragma once
#include <nany/nany.h>


namespace ny {
namespace print {


//! Help usage (always return EXIT_SUCCESS)
int usage(const char* const argv0);

//! Complain that no script has been provided (always return EXIT_FAILURE)
int noInputScript(const char* const argv0);

//! Report to std::cout various useful informations to report a bug
int bugReportInfo(); // always returns EXIT_SUCCESS

//! Print the version (always returns EXIT_SUCCESS)
int version();

//! Complain about an unknown command line option (always return EXIT_FAILURE)
int unknownOption(const char* const argv0, const char* const name);

//! Callback to complain on missing file
void fileAccessError(const nyproject_t*, nybuild_t*, const char* file, uint32_t length);


} // namespace print
} // namespace ny
