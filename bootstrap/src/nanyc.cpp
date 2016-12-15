#include "nanyc-utils.h"


namespace {


int shortOption(const char* const name, const char* const argv0) {
	if (!strcmp(name, "-h"))
		return ny::print::usage(argv0);
	if (!strcmp(name, "-v"))
		return ny::print::version();
	return ny::print::unknownOption(argv0, name);
}


int longOptions(const char* const name, const char* const argv0) {
	if (!strcmp(name, "--help"))
		return ny::print::usage(argv0);
	if (!strcmp(name, "--version"))
		return ny::print::version();
	if (!strcmp(name, "--bugreport"))
		return ny::print::bugReportInfo();
	return ny::print::unknownOption(argv0, name);
}


} // namespace


int main(int argc, const char** argv) {
	if (!(argc > 1))
		return ny::print::noInputScript(argv[0]);
	nyrun_cf_t runcf;
	nyrun_cf_init(&runcf);
	int firstarg = argc; // end of the list
	for (int i = 1; i < argc; ++i) {
		const char* const carg = argv[i];
		if (carg[0] == '-') {
			if (carg[1] != '-')
				return shortOption(carg, argv[0]);
			if (carg[2] != '\0') { // to handle '--' option
				if (!strcmp(carg, "--verbose"))
					runcf.verbose = nytrue;
				else
					return longOptions(carg, argv[0]);
			}
			else {
				// nothing must interpreted after '--'
				firstarg = i + 1;
				break;
			}
			continue;
		}
		firstarg = i;
		break;
	}
	// execute the script
	int exitstatus = EXIT_FAILURE;
	if (firstarg < argc) {
		runcf.build.on_error_file_eacces = &ny::print::fileAccessError;
		// the new arguments, after removing all command line arguments
		int nargc = argc - firstarg;
		const char** nargv = argv + firstarg;
		// the nany sript to load, which should be the first new argument
		const char* nargv0 = nargv[0];
		--nargc;
		uint32_t pargc = (nargc > 0) ? static_cast<uint32_t>(nargc) : 0;
		const char** pargv = (!pargc ? nullptr : (++nargv));
		exitstatus = nyrun_file_n(&runcf, nargv0, strlen(nargv0), pargc, pargv);
	}
	nyrun_cf_release(&runcf);
	return exitstatus;
}
