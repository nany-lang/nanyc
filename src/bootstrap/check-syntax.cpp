#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include <yuni/io/file.h>
#include <yuni/core/system/console.h>
#include <yuni/core/getopt.h>
#include <yuni/io/directory.h>
#include <yuni/io/directory/info.h>
#include <yuni/datetime/timestamp.h>
#include "nany/nany.h"
#include <algorithm>
#include <yuni/datetime/timestamp.h>
#include <iostream>

using namespace Yuni;





namespace // anonymous
{

	uint FindCommonFolder(const String::Vector& filenames)
	{
		if (not filenames.empty())
		{
			auto& firstElement = filenames[0];
			const char sep = IO::Separator;

			uint pos = 0;
			for (; ; ++pos)
			{
				for (size_t i = 0; i < filenames.size(); ++i)
				{
					auto& str = filenames[i];
					if (pos == firstElement.size())
						return pos;

					if (pos < str.size() and str[pos] != '\0' and str[pos] == firstElement[pos])
						continue;

					// backtrace to the last sep
					while (pos > 0 && firstElement[--pos] != sep)
					{}
					return pos;
				}
			}

			return pos;
		}
		return 0;
	}


	template<uint N>
	class Writer final : NonCopyable<Writer<N>>
	{
	public:
		Writer(String& text, nyreport_t* report)
			: text(text), report(report)
		{
			text.clear();
		}
		Writer(Writer&& rhs)
			: text(rhs.text), report(rhs.report)
		{
			text.clear();
		}

		~Writer()
		{
			if (not text.empty())
			{
				switch (N)
				{
					case 0: nany_info(report, text.c_str()); break;
					case 1: nany_warning(report, text.c_str()); break;
					case 2: nany_error(report, text.c_str()); break;
				}
			}
		}


		template<class T> Writer& operator << (const T& value)
		{
			text.append(value);
			return *this;
		}

	private:
		String& text;
		nyreport_t* report;
	};

	class Report final
	{
	public:
		Report()
		{
			report = nany_report_create();
		}

		~Report()
		{
			nany_report_print_stdout(report);
			nany_report_unref(&report);
		}

		Writer<0> info() { return Writer<0>{text, report}; }
		Writer<1> warning() { return Writer<1>{text, report}; }
		Writer<2> error() { return Writer<2>{text, report}; }


	private:
		nyreport_t* report;
		String text;
	};


	static inline bool ExpandFilelist(Report& logs, std::vector<String>& list)
	{
		String::Vector filelist;
		filelist.reserve(512);
		bool success = true;
		String currentfile;

		for (auto& element: list)
		{
			IO::Canonicalize(currentfile, element);
			switch (IO::TypeOf(currentfile))
			{
				case IO::typeFile:
				{
					filelist.emplace_back(currentfile);
					break;
				}

				case IO::typeFolder:
				{
					IO::Directory::Info info(currentfile);
					IO::Directory::Info::recursive_file_iterator i   = info.recursive_file_begin();
					IO::Directory::Info::recursive_file_iterator end = info.recursive_file_end();
					// extension
					ShortString16 ext;

					for (; i != end; ++i)
					{
						IO::ExtractExtension(ext, *i);
						if (ext == ".ny")
							filelist.emplace_back(i.filename());
					}
					break;
				}

				default:
				{
					logs.error() << "impossible to find '" << currentfile << "'";
					success = false;
					break;
				}
			}
		}

		// for beauty in singled-threaded
		std::sort(filelist.begin(), filelist.end());
		list.swap(filelist);
		return success;
	}


	template<class F>
	static inline bool IterateThroughAllFiles(Report& logs, const String::Vector& filenames, const F& callback)
	{
		String currentfile;
		uint testOK = 0;
		uint testFAILED = 0;

		sint64 maxCheckDuration = 0;
		// start time
		sint64 startTime = DateTime::NowMilliSeconds();

		for (auto& filename: filenames)
		{
			sint64 duration = 0;
			if (callback(filename, duration))
				++testOK;
			else
				++testFAILED;

			if (duration > maxCheckDuration)
				maxCheckDuration = duration;
		}

		sint64 endTime = DateTime::NowMilliSeconds();


		uint total = testOK + testFAILED;
		if (total > 0)
		{
			sint64 duration = (endTime - startTime);

			String durationStr;
			durationStr << " (in " << duration << "ms, max: " << maxCheckDuration << "ms)";


			if (0 != testOK)
			{
				switch (total)
				{
					case 1:
						logs.warning() << "fail: 1 file, +" << testOK << ", -" << testFAILED << durationStr;
						break;
					default:
						logs.warning() << "fail: " << total << " files, +" << testOK << ", -" << testFAILED << durationStr;
				}
			}
			else
			{
				switch (total)
				{
					case 1:
						logs.info() << "success: 1 file, +" << testOK << ", -" << testFAILED << durationStr;
						break;
					default:
						logs.info() << "success: " << total << " files, +" << testOK << ", -" << testFAILED << durationStr;
				}
			}
		}
		else
			logs.warning() << "no input file";

		return (0 == testFAILED);
	}



	static bool batchCheckIfFilenamesConformToGrammar(String::Vector& filenames, bool printAll)
	{
		Report logs;
		if (not ExpandFilelist(logs, filenames))
			return false;

		auto commonFolder = (filenames.size() > 1 ? FindCommonFolder(filenames) : 0);
		if (commonFolder != 0)
			++commonFolder;

		return IterateThroughAllFiles(logs, filenames, [&](const AnyString& filename, sint64& duration) -> bool
		{
			sint64 start = DateTime::NowMilliSeconds();
			bool success = nany_utility_validator_check_file_n(filename.c_str(), filename.size());
			duration = DateTime::NowMilliSeconds() - start;

			String barefilename;
			IO::ExtractFileName(barefilename, filename);
			bool expected = (barefilename.startsWith("ok-"));
			success = (success == expected);

			if (success and duration < 300)
			{
				if (printAll)
					logs.info() << AnyString{filename, commonFolder} << " [" << duration << "ms]";
				return true;
			}
			else
			{
				if (not success)
					logs.error() << AnyString{filename, commonFolder} << " [" << duration << "ms]";
				else
					logs.warning() << AnyString{filename, commonFolder} << " [" << duration << "ms - time limit reached]";
			}
			return false;
		});
	}


} // anonymous namespace






int main(int argc, char** argv)
{
	// all input filenames
	String::Vector filenames;
	// errors only
	bool printAll = false;
	// no colors
	bool noColors = false;


	// parse the command
	{
		// The command line options parser
		GetOpt::Parser options;

		// Input files
		options.add(filenames, 'i', "input", "Input files (or folders)");
		options.remainingArguments(filenames);

		// --no-color
		options.addFlag(noColors, ' ', "no-color", "Disable color output");

		// errors only
		options.addFlag(printAll, ' ', "print-all", "Print all tests");

		// HELP
		// help
		options.addParagraph("\nHelp\n");

		// version
		bool optVersion = false;
		options.addFlag(optVersion, ' ', "version", "Display the version of the compiler and exit");

		// Ask to the parser to parse the command line
		if (not options(argc, argv))
		{
			// The program should not continue here
			// The user may have requested the help or an error has happened
			// If an error has happened, the exit status should be different from 0
			if (options.errors())
			{
				std::cerr << "Abort due to error\n";
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}

		if (optVersion)
		{
			std::cout << "0.0\n";
			return EXIT_SUCCESS;
		}

		if (filenames.empty())
		{
			std::cerr << argv[0] << ": no input file\n";
			return EXIT_FAILURE;
		}
	}


	// Print AST or check for Nany Grammar
	bool success = batchCheckIfFilenamesConformToGrammar(filenames, printAll);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

