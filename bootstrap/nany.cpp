#include <yuni/yuni.h>
#include <yuni/string.h>
#include <nany/nany.h>
#include <iostream>
#include <cassert>

using namespace Yuni;




static int printNoInputScript(const char* argv0)
#include <iostream>
{
	std::cerr << argv0 << ": no input script file\n";
	return EXIT_FAILURE;
}


static int printUsage(const char* const argv0)
{
	std::cout
		<< "Usage: " << argv0 << " [options] file...\n"
		<< "Options:\n"
		<< "  --bugreport       Display some useful information to report a bug\n"
		<< "                    (https://github.com/nany-lang/nany/issues/new)\n"
		<< "  --help, -h        Display this information\n"
		<< "  --version, -v     Print the version\n\n";
	return EXIT_SUCCESS;
}


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


static int unknownOption(const char* argv0, const AnyString& name)
{
	std::cerr << argv0 << ": unknown option '" << name << "'\n";
	return EXIT_FAILURE;
}


static void on_error_file_eacces(const nyproject_t*, nybuild_t*, const char* file, uint32_t length)
{
	std::cerr << "error: failed to access to '";
	std::cerr << AnyString{file, length} << "'\n";
}




int main(int argc, const char** argv)
{
	const char* const argv0 = argv[0];

	if (argc > 1)
	{
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
							return printUsage(argv[0]);
						if (arg == "--version")
							return printVersion();
						if (arg == "--bugreport")
							return printBugReportInfo();
						if (arg == "--verbose")
						{
							runcf.verbose = nytrue;
							continue;
						}
						return unknownOption(argv0, arg);
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
						return printUsage(argv[0]);
					if (arg == "-v")
						return printVersion();
					return unknownOption(argv0, arg);
				}
			}

			firstarg = i;
			break;
		}


		//
		// -- execute the script
		//
		int exitstatus = -1;
		if (firstarg < argc)
		{
			// callbacks
			runcf.build.on_error_file_eacces = &on_error_file_eacces;

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

	return printNoInputScript(argv0);
}
