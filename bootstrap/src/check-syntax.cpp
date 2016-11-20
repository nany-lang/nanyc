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


namespace {


struct Settings {
	//! List of filenames to verify
	std::vector<String> filenames;
	// no colors
	bool noColors = false;
	// Result expected from filename convention
	bool useFilenameConvention = false;
};


template<class LeftType = Logs::NullDecorator>
struct ParseVerbosity: public LeftType {
	template<class Handler, class VerbosityType, class O>
	void internalDecoratorAddPrefix(O& out, const AnyString& s) const {
		// Write the verbosity to the output
		if (VerbosityType::hasName) {
			AnyString name{VerbosityType::Name()};
			if (s.empty()) {
			}
			else if (name == "info")
				printInfo<Handler>(out);
			else if (name == "error")
				printError<Handler>(out);
			else if (name == "warning")
				printWarning<Handler>(out);
			else
				printOtherVerbosity<Handler, VerbosityType>(out);
		}
		// Transmit the message to the next decorator
		LeftType::template internalDecoratorAddPrefix<Handler, VerbosityType,O>(out, s);
	}

	template<class Handler, class O>
	static void printInfo(O& out) {
		if (Handler::colorsAllowed)
			System::Console::TextColor<System::Console::yellow>::Set(out);
		#ifndef YUNI_OS_WINDOWS
		out << "  \u2713   ";
		#else
		out << " ok   ";
		#endif
		if (Handler::colorsAllowed) {
			System::Console::ResetTextColor(out);
			System::Console::TextColor<System::Console::bold>::Set(out);
		}
		out << "parsing";
		if (Handler::colorsAllowed)
			System::Console::ResetTextColor(out);
	}

	template<class Handler, class O>
	static void printError(O& out) {
		if (Handler::colorsAllowed)
			System::Console::TextColor<System::Console::red>::Set(out);
		out << " ERR  ";
		if (Handler::colorsAllowed) {
			System::Console::ResetTextColor(out);
			System::Console::TextColor<System::Console::bold>::Set(out);
		}
		out << "parsing";
		if (Handler::colorsAllowed)
			System::Console::ResetTextColor(out);
	}

	template<class Handler, class O>
	static void printWarning(O& out) {
		if (Handler::colorsAllowed)
			System::Console::TextColor<System::Console::yellow>::Set(out);
		out << " warn ";
		if (Handler::colorsAllowed) {
			System::Console::ResetTextColor(out);
			System::Console::TextColor<System::Console::bold>::Set(out);
		}
		out << "parsing";
		if (Handler::colorsAllowed)
			System::Console::ResetTextColor(out);
	}

	template<class Handler, class VerbosityType, class O>
	static void printOtherVerbosity(O& out) {
		// Set Color
		if (Handler::colorsAllowed && VerbosityType::color != System::Console::none)
			System::Console::TextColor<VerbosityType::color>::Set(out);
		// The verbosity
		VerbosityType::AppendName(out);
		// Reset Color
		if (Handler::colorsAllowed && VerbosityType::color != System::Console::none)
			System::Console::ResetTextColor(out);
	}

}; // struct VerbosityLevel


using Logging = Logs::Logger<Logs::StdCout<>, ParseVerbosity<Logs::Message<>>>;
static Logging logs;


uint32_t findCommonFolderLength(const std::vector<String>& filenames) {
	if (filenames.empty())
		return 0;
	uint32_t len = 0;
	auto& firstElement = filenames[0];
	const char sep = IO::Separator;
	for (; ; ++len) {
		for (auto& filename: filenames) {
			if (len == firstElement.size())
				return len;
			if (len < filename.size() and filename[len] != '\0' and filename[len] == firstElement[len])
				continue;
			while (len > 0 and firstElement[--len] != sep) { // back to the last sep
			}
			return len;
		}
	}
	return len;
}

bool expandAndCanonicalizeFilenames(std::vector<String>& filenames) {
	std::vector<String> filelist;
	filelist.reserve(512);
	String currentfile;
	currentfile.reserve(4096);
	for (auto& filename: filenames) {
		IO::Canonicalize(currentfile, filename);
		switch (IO::TypeOf(currentfile)) {
			case IO::typeFile: {
				filelist.emplace_back(currentfile);
				break;
			}
			case IO::typeFolder: {
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
			default: {
				logs.error() << "impossible to find '" << currentfile << "'";
				return false;
			}
		}
	}
	// for beauty in singled-threaded (and to always produce the same output)
	std::sort(filelist.begin(), filelist.end());
	filenames.swap(filelist);
	return true;
}

template<class F>
bool IterateThroughAllFiles(const std::vector<String>& filenames, const F& callback) {
	String currentfile;
	uint32_t testOK = 0;
	uint32_t testFAILED = 0;
	int64_t maxCheckDuration = 0;
	int64_t startTime = DateTime::NowMilliSeconds();
	for (auto& filename: filenames) {
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
	if (total > 0) {
		int64_t duration = (endTime - startTime);
		String durationStr;
		durationStr << " (in " << duration << "ms, max: " << maxCheckDuration << "ms)";
		if (total > 1) {
			if (0 != testFAILED) {
				switch (total) {
					case 1:
						logs.warning() << "-- FAILED -- 1 file, +" << testOK << ", -" << testFAILED << durationStr;
						break;
					default:
						logs.warning() << "-- FAILED -- " << total << " files, +" << testOK << ", -" << testFAILED << durationStr;
				}
			}
			else
			{
				switch (total) {
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

bool batchCheckIfFilenamesConformToGrammar(Settings& settings) {
	if (not expandAndCanonicalizeFilenames(settings.filenames))
		return false;
	auto commonFolder = (settings.filenames.size() > 1 ? findCommonFolderLength(settings.filenames) : 0);
	if (0 != commonFolder)
		++commonFolder;
	return IterateThroughAllFiles(settings.filenames, [&](const AnyString& file, int64_t& duration) -> bool {
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
		// PARSE
		int64_t start = DateTime::NowMilliSeconds();
		bool success = (nytrue == nytry_parse_file_n(file.c_str(), file.size()));
		duration = DateTime::NowMilliSeconds() - start;
		success = (success == expected);
		if (success and duration < 300) {
			logs.info() << AnyString{file, commonFolder} << " [" << duration << "ms]";
		}
		else {
			if (not success) {
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

bool parseCommandLine(Settings& settings, int argc, char** argv) {
	GetOpt::Parser options;
	options.add(settings.filenames, 'i', "input", "Input files (or folders)");
	options.remainingArguments(settings.filenames);
	options.addFlag(settings.noColors, ' ', "no-color", "Disable color output");
	options.addFlag(settings.useFilenameConvention, ' ', "use-filename-convention",
		"Use the filename to determine if the test should succeed or not (should succeed if starting with 'ok-'");
	bool optVersion = false;
	options.addFlag(optVersion, ' ', "version", "Display the version of the compiler and exit");
	if (not options(argc, argv)) {
		// The program should not continue here
		// The user may have requested the help or an error has happened
		// If an error has happened, the exit status should be different from 0
		if (options.errors())
			throw std::runtime_error("Abort due to error");
		return false;
	}
	if (optVersion) {
		std::cout << "0.0\n";
		return false;
	}
	if (settings.filenames.empty())
		throw std::runtime_error("no input file");
	return true;
}

} // namespace


int main(int argc, char** argv)
{
	try {
		Settings settings;
		if (not parseCommandLine(settings, argc, argv))
			return EXIT_SUCCESS;
		bool success = batchCheckIfFilenamesConformToGrammar(settings);
		return success ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	catch (const std::exception& e) {
		std::cerr << argv[0] << ": " << e.what() << '\n';
	}
	return EXIT_FAILURE;
}
