#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include <yuni/io/file.h>
#include <yuni/core/system/console.h>
#include <yuni/core/getopt.h>
#include <yuni/io/directory.h>
#include <yuni/io/directory/info.h>
#include <yuni/datetime/timestamp.h>
#include <yuni/core/logs/logs.h>
#include "nany/nany.h"
#include <algorithm>
#include <yuni/datetime/timestamp.h>
#include <iostream>
#include <vector>

using namespace Yuni;


struct
{
	// no colors
	bool noColors = false;
	// Result expected from filename convention
	bool useFilenameConvention = false;
}
settings;




template<class LeftType = Logs::NullDecorator>
class ParseVerbosity : public LeftType
{
public:
	template<class Handler, class VerbosityType, class O>
	void internalDecoratorAddPrefix(O& out, const AnyString& s) const
	{
		// Write the verbosity to the output
		if (VerbosityType::hasName)
		{
			AnyString name{VerbosityType::Name()};

			if (s.empty())
			{
			}
			else if (name == "info")
			{
				if (Handler::colorsAllowed)
					System::Console::TextColor<System::Console::yellow>::Set(out);
				#ifndef YUNI_OS_WINDOWS
				out << "      \u2713  ";
				#else
				out << "      >  ";
				#endif

				if (Handler::colorsAllowed)
					System::Console::TextColor<System::Console::white>::Set(out);
				out << "parsing";

				if (Handler::colorsAllowed)
					System::Console::ResetTextColor(out);
			}
			else if (name == "error")
			{
				if (Handler::colorsAllowed)
					System::Console::TextColor<System::Console::red>::Set(out);
				out << "  FAILED ";

				if (Handler::colorsAllowed)
					System::Console::TextColor<System::Console::white>::Set(out);
				out << "parsing";

				if (Handler::colorsAllowed)
					System::Console::ResetTextColor(out);
			}
			else if (name == "warning")
			{
				if (Handler::colorsAllowed)
					System::Console::TextColor<System::Console::yellow>::Set(out);
				out << "  {warn} ";

				if (Handler::colorsAllowed)
					System::Console::TextColor<System::Console::white>::Set(out);
				out << "parsing";

				if (Handler::colorsAllowed)
					System::Console::ResetTextColor(out);
			}

			else
			{
				// Set Color
				if (Handler::colorsAllowed && VerbosityType::color != System::Console::none)
					System::Console::TextColor<VerbosityType::color>::Set(out);
				// The verbosity
				VerbosityType::AppendName(out);
				// Reset Color
				if (Handler::colorsAllowed && VerbosityType::color != System::Console::none)
					System::Console::ResetTextColor(out);
			}
		}
		// Transmit the message to the next decorator
		LeftType::template internalDecoratorAddPrefix<Handler, VerbosityType,O>(out, s);
	}
}; // class VerbosityLevel


typedef Logs::Logger<Logs::StdCout<>,
		ParseVerbosity<Logs::Message<>> > Logging;
static Logging  logs;




static uint32_t fincCommonFolderLength(const std::vector<String>& filenames)
{
	if (filenames.empty())
		return 0;

	auto& firstElement = filenames[0];
	const char sep = IO::Separator;

	uint32_t pos = 0;
	for (; ; ++pos)
	{
		for (size_t i = 0; i < filenames.size(); ++i)
		{
			auto& str = filenames[i];
			if (pos == firstElement.size())
				return pos;

			if (pos < str.size() and str[pos] != '\0' and str[pos] == firstElement[pos])
				continue;

			// back to the last sep
			while (pos > 0 && firstElement[--pos] != sep)
			{}
			return pos;
		}
	}

	return pos;
}


static bool expandFilelist(std::vector<String>& list)
{
	std::vector<String> filelist;
	filelist.reserve(512);
	String currentfile;
	currentfile.reserve(4096);

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
				ShortString16 ext;

				IO::Directory::Info info(currentfile);
				auto end = info.recursive_file_end();
				for (auto i = info.recursive_file_begin(); i != end; ++i)
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
				return false;
			}
		}
	}

	// for beauty in singled-threaded (and to always produce the same output)
	std::sort(filelist.begin(), filelist.end());
	list.swap(filelist);
	return true;
}


template<class F>
static bool IterateThroughAllFiles(const std::vector<String>& filenames, const F& callback)
{
	String currentfile;
	uint32_t testOK = 0;
	uint32_t testFAILED = 0;
	int64_t maxCheckDuration = 0;

	int64_t startTime = DateTime::NowMilliSeconds();
	for (auto& filename: filenames)
	{
		int64_t duration = 0;
		if (callback(filename, duration))
			++testOK;
		else
			++testFAILED;

		if (duration > maxCheckDuration)
			maxCheckDuration = duration;
	}
	int64_t endTime = DateTime::NowMilliSeconds();

	uint32_t total = testOK + testFAILED;
	if (total > 0)
	{
		int64_t duration = (endTime - startTime);

		String durationStr;
		durationStr << " (in " << duration << "ms, max: " << maxCheckDuration << "ms)";

		if (total > 1)
		{
			if (0 != testFAILED)
			{
				switch (total)
				{
					case 1:
						logs.warning() << "-- FAILED -- 1 file, +" << testOK << ", -" << testFAILED << durationStr;
						break;
					default:
						logs.warning() << "-- FAILED -- " << total << " files, +" << testOK << ", -" << testFAILED << durationStr;
				}
			}
			else
			{
				switch (total)
				{
					case 1:
						logs.info() << "success: 1 file, +" << testOK << ", -0" << durationStr;
						break;
					default:
						logs.info() << "success: " << total << " files, +" << testOK << ", -0" << durationStr;
				}
			}
		}
	}
	else
		logs.warning() << "no input file";

	return (0 == testFAILED);
}



static bool batchCheckIfFilenamesConformToGrammar(std::vector<String>& filenames)
{
	if (not expandFilelist(filenames))
		return false;

	auto commonFolder = (filenames.size() > 1 ? fincCommonFolderLength(filenames) : 0);
	if (0 != commonFolder)
		++commonFolder;

	return IterateThroughAllFiles(filenames, [&](const AnyString& file, int64_t& duration) -> bool
	{
		String barefile;
		IO::ExtractFileName(barefile, file);

		bool expected = true;
		bool canfail = false;
		if (settings.useFilenameConvention)
		{
			if (barefile.startsWith("ko-"))
				expected = false;
			if (barefile.find("-canfail-") < barefile.size())
				canfail = true;
		}

		//
		// -- PARSE --
		//
		int64_t start = DateTime::NowMilliSeconds();
		bool success = (nytrue == nany_try_parse_file_n(file.c_str(), file.size()));
		duration = DateTime::NowMilliSeconds() - start;

		success = (success == expected);

		if (success and duration < 300)
		{
			logs.info() << AnyString{file, commonFolder} << " [" << duration << "ms]";
		}
		else
		{
			if (not success)
			{
				if (not canfail)
					logs.error() << AnyString{file, commonFolder} << " [" << duration << "ms]";
				else
					logs.warning() << AnyString{file, commonFolder} << " [" << duration << "ms, can fail]";
			}
			else
				logs.error() << AnyString{file, commonFolder} << " [" << duration << "ms - time limit reached]";
			success = canfail;
		}
		return success;
	});
}






int main(int argc, char** argv)
{
	// all input filenames
	std::vector<String> filenames;
	// parse the command
	{
		// The command line options parser
		GetOpt::Parser options;

		// Input files
		options.add(filenames, 'i', "input", "Input files (or folders)");
		options.remainingArguments(filenames);

		// --no-color
		options.addFlag(settings.noColors, ' ', "no-color", "Disable color output");

		// use filename convention
		options.addFlag(settings.useFilenameConvention, ' ', "use-filename-convention",
			"Use the filename to determine if the test should succeed or not (should succeed if starting with 'ok-'");

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
	bool success = batchCheckIfFilenamesConformToGrammar(filenames);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
