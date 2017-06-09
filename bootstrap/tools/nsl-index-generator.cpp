#include <yuni/yuni.h>
#include <yuni/io/file.h>
#include <yuni/core/getopt.h>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace yuni;

namespace {

void splitAndSort(std::vector<String>& array, const AnyString& strlist) {
	strlist.words(";", [&](const AnyString& item) -> bool {
		array.emplace_back(item);
		return true;
	});
	std::sort(std::begin(array), std::end(array));
}

void sanitizeContent(Clob& content) {
	content.trimRight();
	content.replace("\\", "\\\\");
	content.replace("\"", "\\\"");
	content.replace("\n", "\\n");
	content.replace("\t", "\\t");
}


class Generator final {
public:
	std::vector<String> filenames;
	std::vector<String> unittests;
	Clob hxx;
	AnyString output;
	AnyString rootdir;
	Clob filecontent; // for loading file contents

	Generator() {
		filenames.reserve(64); // arbitrary
		unittests.reserve(64);
		filecontent.reserve(64 * 1024);
		hxx.reserve(128 * 1024);
	}

	void parse(int argc, char* argv[]) {
		AnyString allfilenames;
		AnyString allunits;
		GetOpt::Parser options;
		options.add(allfilenames, 'f', "", "Nsl core file (';' separated)");
		options.add(allunits, 'u', "", "Unittest files (';' separated)");
		options.add(output, 'o', "output", "Output file");
		options.add(rootdir, 'd', "dir", "Root directory (for pretty filenames)");
		if (not options(argc, argv)) {
			if (options.errors()) {
				std::cerr << "Abort due to error\n";
				throw EXIT_FAILURE;
			}
			throw EXIT_SUCCESS;
		}
		splitAndSort(filenames, allfilenames);
		splitAndSort(unittests, allunits);
	}

	void writeSetOfFiles(const AnyString& funcname, const std::vector<String>& files) {
		AnyString prefix = "{nsl} ";
		hxx << "template<class T, class C> void " << funcname << "(T& allsources, uint32_t& i, C&& callback) {\n";
		for (auto& filename: files) {
			auto err = IO::File::LoadFromFile(filecontent, filename);
			if (err != IO::errNone)
				throw String("eaccess: ") << filename;
			sanitizeContent(filecontent);
			AnyString prettyname = filename;
			prettyname.consume(rootdir.size() + 1);
			hxx << "	{\n";
			hxx << "		auto& source = allsources[i];\n";
			hxx << "		source.filename.adapt(\"" << prefix << prettyname << "\", " << (prettyname.size() + prefix.size()) << ");\n";
			hxx << "		source.content.adapt(\"" << filecontent << "\", " << filecontent.size() << ");\n";
			hxx << "		callback(source);\n";
			hxx << "		++i;\n";
			hxx << "	}\n";
		}
		hxx << "}\n";
	}

	void build() {
		hxx << "#pragma once\n";
		hxx << '\n';
		hxx << "// !!! FILE AUTOMATICALLY GENERATED !!!\n";
		hxx << '\n';
		hxx << "namespace ny {\n";
		hxx << "namespace compiler {\n";
		hxx << "namespace {\n";
		hxx << '\n';
		hxx << "constexpr uint32_t corefilesCount = " << filenames.size() << ";\n";
		hxx << "constexpr uint32_t unittestCount = " << unittests.size() << ";\n";
		hxx << '\n';
		writeSetOfFiles("registerNSLCoreFiles", filenames);
		hxx << '\n';
		writeSetOfFiles("registerUnittestFiles", unittests);
		hxx << '\n';
		hxx << "} // namespace\n";
		hxx << "} // namespace compiler\n";
		hxx << "} // namespace ny\n";
	}

	void write() {
		std::cout << "[bootstrap/embed-nsl] +" << filenames.size() << " nsl files";
		std::cout << ", +" << unittests.size() << " unittests files\n";
		std::cout << "[bootstrap/embed-nsl] writing " << output << " (" << hxx.size() << " bytes)\n";
		String folder;
		IO::ExtractFilePath(folder, output);
		if (not IO::Exists(folder)) {
			if (not IO::Directory::Create(folder))
				throw String("failed to create directory ") << folder;
		}
		bool written = IO::File::SetContent(output, hxx);
		if (not written)
			throw String("failed to write ") << output;
	}
};

} // namespace

int main(int argc, char* argv[]) {
	try {
		Generator generator;
		generator.parse(argc, argv);
		generator.build();
		generator.write();
		return EXIT_SUCCESS;
	}
	catch (const String& e) {
		std::cerr << "error: " << e << '\n';
	}
	catch (const std::exception& e) {
		std::cerr << "exception: " << e.what() << '\n';
	}
	catch (int e) {
		return e;
	}
	return EXIT_FAILURE;
}
