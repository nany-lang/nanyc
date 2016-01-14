#include <yuni/yuni.h>
#include <yuni/string.h>
#include <yuni/core/system/main.h>
#include <yuni/io/file.h>
#include <nany/nany.h>
#include <memory>
#include <iostream>
#include <cassert>
#include <limits.h>

using namespace Yuni;

static const char* argv0 = "";

static bool verbose = false;



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
	auto* text = nany_get_info_for_bugreport();
	if (text)
	{
		std::cout << AnyString{text};
		::free(text);
		return 0;
	}
	return EXIT_FAILURE;
}


static int printVersion()
{
	std::cout << nany_version() << '\n';
	return EXIT_SUCCESS;
}


static int unknownOption(const AnyString& name)
{
	std::cerr << argv0 << ": unknown option '" << name << "'\n";
	return EXIT_FAILURE;
}



static inline void printReport(const nyreport_t* report, bool status)
{
	(status ? nany_report_print_stdout : nany_report_print_stderr)(report);
}


static inline bool buildProgram(nycontext_t& ctx)
{
	nyreport_t* report = nullptr;
	nybool_t buildstatus = nany_build(&ctx, &report);
	bool success = (buildstatus == nytrue);

	// report printing
	// if an error has occured or if in verbose mode
	if (YUNI_UNLIKELY((not success) or verbose))
		printReport(report, success);

	nany_report_unref(&report);
	return success;
}


static inline int execute(int argc, char** argv)
{
	assert(argc >= 1);
	String scriptfile;
	IO::Canonicalize(scriptfile, argv[0]);

	// nany context
	nycontext_t ctx;
	nany_initialize(&ctx, nullptr);

	// for concurrent build
	//ctx.mt.queueservice = nany_queueservice_create();

	nany_source_add_from_file_n(&ctx, scriptfile.c_str(), scriptfile.size());

	bool buildstatus = buildProgram(ctx);
	int exitstatus = 66;

	if (buildstatus)
	{
		if (argc == 1)
		{
			const char* nargv[] = { scriptfile.c_str(), nullptr };
			exitstatus = nany_run_main(&ctx, 1, nargv);
		}
		else
		{
			const char** wptr = (const char**)::malloc(((size_t) argc + 1) * sizeof(const char**));
			if (wptr)
			{
				auto nargv = std::unique_ptr<const char*, decltype(free)*>{wptr, ::free};
				for (int i = 1; i < argc; ++i)
					nargv.get()[i] = argv[i];
				nargv.get()[0]    = scriptfile.c_str();
				nargv.get()[argc] = nullptr;

				exitstatus = nany_run_main(&ctx, argc, nargv.get());
			}
		}
	}

	nany_uninitialize(&ctx);
	return exitstatus;
}







YUNI_MAIN_CONSOLE(argc, argv)
{
	argv0 = argv[0];
	if (YUNI_UNLIKELY(argc <= 1))
	{
		std::cerr << argv0 << ": no input script file\n";
		return EXIT_FAILURE;
	}

	try
	{
		int firstarg = -1;
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
							verbose = true;
							continue;
						}
						return unknownOption(arg);
					}
					else
					{
						firstarg = i + 1;
						break; // nothing must interpreted after '--'
					}
				}
				else
				{
					AnyString arg{carg};
					if (arg == "-h")
						return printUsage(argv[0]);
					if (arg == "-v")
						return printVersion();
					return unknownOption(arg);
				}
			}

			firstarg = i;
			break;
		}

		//
		// -- execute the script
		//
		return execute(argc - firstarg, argv + firstarg);
	}
	catch (std::bad_alloc&)
	{
		std::cerr << '\n' << argv0 << ": error: failed to allocate memory\n";
	}
	catch (...)
	{
		std::cerr << '\n' << argv0 << ": error: unhandled exception\n";
	}
	return EXIT_FAILURE;
}
