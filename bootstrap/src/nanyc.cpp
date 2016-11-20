#include "nanyc-utils.h"


int main(int argc, const char** argv)
{
	if (!(argc > 1))
		return ny::print::noInputScript(argv[0]);
	nyrun_cf_t runcf;
	nyrun_cf_init(&runcf);
	int firstarg = argc; // end of the list
	for (int i = 1; i < argc; ++i)
	{
		const char* const carg = argv[i];
		if (carg[0] == '-')
		{
			if (carg[1] == '-')
			{
				if (carg[2] != '\0') // to handle '--' option
				{
					AnyString arg{carg};

					if (arg == "--help")
						return ny::print::usage(argv[0]);
					if (arg == "--version")
						return ny::print::version();
					if (arg == "--bugreport")
						return ny::print::bugReportInfo();
					if (arg == "--verbose")
					{
						runcf.verbose = nytrue;
						continue;
					}
					return ny::print::unknownOption(argv[0], arg);
				}
				else
				{
					// nothing must interpreted after '--'
					firstarg = i + 1;
					break;
				}
			}
			else
			{
				AnyString arg{carg};
				if (arg == "-h")
					return ny::print::usage(argv[0]);
				if (arg == "-v")
					return ny::print::version();
				return ny::print::unknownOption(argv[0], arg);
			}
		}
		firstarg = i;
		break;
	}
	// execute the script
	int exitstatus = -1;
	if (firstarg < argc)
	{
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
