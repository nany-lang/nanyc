#include <yuni/yuni.h>
#include <yuni/io/file.h>
#include <yuni/core/system/console.h>
#include <yuni/core/getopt.h>
#include "details/grammar/nany.h"
#include <iostream>

using namespace Yuni;



static bool  printAST(const AnyString filename, bool unixcolors)
{
	if (filename.empty())
		return false;

	Nany::Parser parser;
	if (not parser.loadFromFile(filename))
		return false;

	if (nullptr == parser.root)
		return false;

	Clob out;
	Nany::Node::Export(out, *parser.root, unixcolors);

	std::cout.write(out.c_str(), out.size());
	return true;
}




int main(int argc, char** argv)
{
	// all input filenames
	String::Vector filenames;
	// no colors
	bool noColors = false;


	// parse the command
	{
		// The command line options parser
		GetOpt::Parser options;

		// Input files
		options.add(filenames, 'i', "input", "Input files (or folders)");
		options.remainingArguments(filenames);


		// OUTPUT
		options.addParagraph("\nOutput\n");
		// --no-color
		options.addFlag(noColors, ' ', "no-color", "Disable color output");

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


	// colors
	bool withColors = ((not noColors) and System::Console::IsStdoutTTY());

	for (auto& path: filenames)
		printAST(path, withColors);

	return EXIT_SUCCESS;
}

