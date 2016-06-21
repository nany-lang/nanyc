#include <yuni/yuni.h>
#include <yuni/string.h>
#include <yuni/core/system/main.h>
#include <yuni/core/system/environment.h>
#include <yuni/io/file.h>
#include <nany/nany.h>
#include <memory>
#include <iostream>
#include <cassert>
#include <limits.h>
#include <memory>

using namespace Yuni;

static const char* argv0 = "";

#define likely(X)    YUNI_LIKELY(X)
#define unlikely(X)  YUNI_UNLIKELY(X)




namespace std
{
	template<> struct default_delete<nyproject_t> final
	{
		inline void operator () (nyproject_t* ptr)
		{
			nany_project_unref(ptr);
		}
	};

	template<> struct default_delete<nybuild_t> final
	{
		inline void operator () (nybuild_t* ptr)
		{
			nany_build_unref(ptr);
		}
	};

	template<> struct default_delete<nyprogram_t> final
	{
		inline void operator () (nyprogram_t* ptr)
		{
			nany_program_unref(ptr);
		}
	};

} // namespace std



namespace // anonymous
{

	struct Options final
	{
		//! Verbose mode
		bool verbose = false;
		//! Maximum number of jobs
		uint32_t jobs = 1;
		//! Memory limit (zero means unlimited)
		size_t memoryLimit = 0;
		//! The new argv0
		String argv0;
	};


	static int printNoInputScript()
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
		nany_print_info_for_bugreport();
		return EXIT_SUCCESS;
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


	/*!
	** \brief Create a new nany project
	*/
	static inline std::unique_ptr<nyproject_t> createProject(const Options& options)
	{
		nyproject_cf_t cf;
		nany_project_cf_init(&cf);

		size_t limit = options.memoryLimit;
		if (unlikely(limit == 0))
			limit = static_cast<size_t>(System::Environment::ReadAsUInt64("NANY_MEMORY_LIMIT"));

		if (unlikely(limit != 0))
			nany_memalloc_set_with_limit(&cf.allocator, limit);

		return std::unique_ptr<nyproject_t>{nany_project_create(&cf)};
	}


	/*!
	** \brief Try to compile the input script filename
	*/
	static inline std::unique_ptr<nyprogram_t> compile(const AnyString& argv0, Options& options)
	{
		// PROJECT
		auto project = createProject(options);
		if (unlikely(!project))
			return nullptr;

		// SOURCE
		auto& file = options.argv0;
		IO::Canonicalize(file, argv0);
		auto r = nany_project_add_source_from_file_n(project.get(), file.c_str(), file.size());
		if (unlikely(r == nyfalse))
			return nullptr;

		// BUILD
		nybuild_cf_t cf;
		nany_build_cf_init(&cf, project.get());
		auto build = std::unique_ptr<nybuild_t>{nany_build_prepare(project.get(), &cf)};
		auto bStatus = nany_build(build.get());
		if (bStatus != nyfalse)
		{
			if (unlikely(options.verbose))
				nany_build_print_report_to_console(build.get(), nytrue);

			nyprogram_cf_t pcf;
			nany_program_cf_init(&pcf, &cf);
			return std::unique_ptr<nyprogram_t>(nany_program_prepare(build.get(), &pcf));
		}

		// an error has occured
		nybool_t addHeader = (options.verbose ? nytrue : nyfalse);
		nany_build_print_report_to_console(build.get(), addHeader);
		return nullptr;
	}


	/*!
	** \brief Execute the script in argv[0]
	**
	** \param argc The total number of arguments
	** \param argv List of UTF8 arguments to pass to the script. The first one is the script to evaluate
	**  (like `main()` for the program)
	** \param options Evaluation options
	** \return Exit status
	*/
	static inline int execute(int argc, char** argv, Options& options)
	{
		assert(argc >= 1);

		auto program = compile(argv[0], options);
		int exitstatus = 66;

		if (!!program)
		{
			if (argc == 1)
			{
				const char* nargv[] = { options.argv0.c_str(), nullptr };
				exitstatus = nany_main(program.get(), 1, nargv);
			}
			else
			{
				auto** nargv = (const char**)::malloc(((size_t) argc + 1) * sizeof(const char**));
				if (nargv)
				{
					nargv[0] = options.argv0.c_str();
					for (int i = 1; i < argc; ++i)
						nargv[i] = argv[i];
					nargv[argc] = nullptr;

					exitstatus = nany_main(program.get(), argc, nargv);
					free(nargv);
				}
			}
		}

		return exitstatus;
	}


} // anonymous namespace




int main(int argc, char** argv)
{
	argv0 = argv[0];

	if (argc > 1)
	{
		Options options;
		int firstarg = argc; // end of the list

		for (int i = 1; i < argc; ++i)
		{
			const char* const carg = argv[i];
			if (unlikely(carg[0] == '-'))
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
							options.verbose = true;
							continue;
						}
						return unknownOption(arg);
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
					return unknownOption(arg);
				}
			}

			firstarg = i;
			break;
		}


		//
		// -- execute the script
		//
		if (firstarg < argc)
			return execute(argc - firstarg, argv + firstarg, options);
	}

	return printNoInputScript();
}
