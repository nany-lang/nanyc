#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/io/file.h>
#include <yuni/core/system/environment.h>
#include "nany/nany.h"
#include <memory>

using namespace Yuni;


#define likely(X)    YUNI_LIKELY(X)
#define unlikely(X)  YUNI_UNLIKELY(X)


//! Arbitrary maximum length (in bytes) of an input source
constexpr static size_t sourceMaxLength = 1024 * 1024 * 1024;
constexpr static int exitFailure = -42;


namespace std {


template<> struct default_delete<nyproject_t> final {
	void operator () (nyproject_t* ptr) { nyproject_unref(ptr); }
};


template<> struct default_delete<nybuild_t> final {
	inline void operator () (nybuild_t* ptr) { nybuild_unref(ptr); }
};


template<> struct default_delete<nyprogram_t> final {
	inline void operator () (nyprogram_t* ptr) { nyprogram_unref(ptr); }
};


} // namespace std


namespace { // anonymous


template<class T>
inline std::unique_ptr<T> make_unique_from_ptr(T* pointer) {
	if (unlikely(!pointer))
		throw std::runtime_error("failed");
	return std::unique_ptr<T>{pointer};
}


//! Create a new nany project
std::unique_ptr<nyproject_t> createProject(const nyrun_cf_t* const runcf) {
	nyproject_cf_t cf;
	if (runcf) {
		memcpy(&(cf), &(runcf->project), sizeof(nyproject_cf_t));
		memcpy(&(cf.allocator), &(runcf->allocator), sizeof(nyallocator_t));
	}
	else
		nyproject_cf_init(&cf);
	return make_unique_from_ptr<nyproject_t>(nyproject_create(&cf));
}


std::unique_ptr<nyprogram_t> build(const nyrun_cf_t* const runcf, std::unique_ptr<nyproject_t> project) {
	if (unlikely(!project))
		return nullptr;
	nybuild_cf_t cf;
	bool verbose;
	if (runcf) {
		verbose = (runcf->verbose != nyfalse);
		memcpy(&(cf),           &(runcf->build),     sizeof(nybuild_cf_t));
		memcpy(&(cf.allocator), &(runcf->allocator), sizeof(nyallocator_t));
		memcpy(&(cf.console),   &(runcf->console),   sizeof(nyconsole_t));
	}
	else {
		verbose = false;
		nybuild_cf_init(&cf, project.get());
	}
	auto buildinfo = make_unique_from_ptr<nybuild_t>(nybuild_prepare(project.get(), &cf));
	auto bStatus = nybuild(buildinfo.get());
	if (unlikely(bStatus == nyfalse)) {
		nybool_t addHeader = (verbose) ? nytrue : nyfalse;
		nybuild_print_report_to_console(buildinfo.get(), addHeader);
		throw std::runtime_error("failed to build");
	}
	if (unlikely(verbose))
		nybuild_print_report_to_console(buildinfo.get(), nytrue);
	nyprogram_cf_t pcf;
	if (runcf) {
		memcpy(&(pcf),           &(runcf->program),   sizeof(nyprogram_cf_t));
		memcpy(&(pcf.allocator), &(runcf->allocator), sizeof(nyallocator_t));
		memcpy(&(pcf.console),   &(runcf->console),   sizeof(nyconsole_t));
	}
	else
		nyprogram_cf_init(&pcf, &cf);
	return make_unique_from_ptr<nyprogram_t>(nyprogram_prepare(buildinfo.get(), &pcf));
}


//! Try to compile the input script filename
std::unique_ptr<nyprogram_t> compileFile(const nyrun_cf_t* cf, const AnyString& argv0, String& file) {
	auto project = createProject(cf);
	IO::Canonicalize(file, argv0);
	auto r = nyproject_add_source_from_file_n(project.get(), file.c_str(), file.size());
	if (r == nyfalse)
		throw std::runtime_error("failed to add source");
	return build(cf, std::move(project));
}


//! Try to compile a list of input files
std::unique_ptr<nyprogram_t> compileFilelist(const nyrun_cf_t* cf, const char** list, uint32_t count) {
	auto project = createProject(cf);
	String filename;
	filename.reserve(1024);
	for (uint32_t i = 0; i != count; ++i) {
		IO::Canonicalize(filename, list[i]);
		auto r = nyproject_add_source_from_file_n(project.get(), filename.c_str(), filename.size());
		if (YUNI_UNLIKELY(r == nyfalse))
			throw std::runtime_error("failed to add source");
	}
	return build(cf, std::move(project));
}


//! Try to compile the input source
std::unique_ptr<nyprogram_t> compileSource(const nyrun_cf_t* cf, const AnyString& source) {
	auto project = createProject(cf);
	auto r = nyproject_add_source_n(project.get(), source.c_str(), source.size());
	if (r == nyfalse)
		throw std::runtime_error("failed to add source");
	return build(cf, std::move(project));
}


int run(nyprogram_t* const program, const char* argv0, uint32_t argc, const char** argv) {
	int exitstatus = exitFailure;
	if (argc == 0 or argv == nullptr) {
		const char* nargv[] = { argv0, nullptr };
		exitstatus = nyprogram_main(program, 1u, nargv);
	}
	else {
		auto** nargv = (const char**)::malloc((argc + 1 + 1) * sizeof(const char**));
		if (nargv) {
			nargv[0] = argv0;
			for (uint32_t i = 0; i != argc; ++i)
				nargv[i + 1] = argv[i];
			nargv[argc + 1] = nullptr;
			exitstatus = nyprogram_main(program, argc, nargv);
			free(nargv);
		}
	}
	return exitstatus;
}


} // anonymous namespace


extern "C" int nyrun_n(const nyrun_cf_t* cf, const char* source, size_t length, uint32_t argc,
					   const char** argv) {
	if (source and length != 0 and length < sourceMaxLength) { // arbitrary
		try {
			auto program = compileSource(cf, AnyString{source, (uint32_t) length});
			if (!!program)
				return run(program.get(), "a.out", argc, argv);
		}
		catch (...) {}
	}
	return exitFailure;
}


extern "C" int nyrun(const nyrun_cf_t* cf, const char* source, uint32_t argc, const char** argv) {
	size_t length = (source) ? strlen(source) : 0;
	if (source and length != 0 and length < sourceMaxLength) { // arbitrary
		try {
			auto program = compileSource(cf, AnyString{source, (uint32_t) length});
			if (!!program)
				return run(program.get(), "a.out", argc, argv);
		}
		catch (...) {}
	}
	return exitFailure;
}


extern "C" int nyrun_file_n(const nyrun_cf_t* cf, const char* file, size_t length, uint32_t argc,
							const char** argv) {
	if (file and length != 0 and length < 32768) {
		try {
			String filename;
			auto program = compileFile(cf, AnyString{file, static_cast<uint32_t>(length)}, filename);
			if (!!program)
				return run(program.get(), filename.c_str(), argc, argv);
		}
		catch (...) {}
	}
	return exitFailure;
}


extern "C" int nyrun_file(const nyrun_cf_t* cf, const char* file, uint32_t argc, const char** argv) {
	size_t length = (file) ? strlen(file) : 0;
	if (file and length != 0 and length < 32768) {
		try {
			String filename;
			auto program = compileFile(cf, AnyString{file, static_cast<uint32_t>(length)}, filename);
			if (!!program)
				return run(program.get(), filename.c_str(), argc, argv);
		}
		catch (...) {}
	}
	return exitFailure;
}


extern "C" int nyrun_filelist(const nyrun_cf_t* cf, const char** files, uint32_t file_count, uint32_t argc,
							  const char** argv) {
	if (files and file_count != 0) {
		try {
			auto program = compileFilelist(cf, files, file_count);
			if (!!program)
				return run(program.get(), "a.out", argc, argv);
		}
		catch (...) {}
	}
	return exitFailure;
}


extern "C" void nyrun_cf_init(nyrun_cf_t* cf) {
	if (cf) {
		// reset the whole struct
		memset(cf, 0x0, sizeof(nyrun_cf_t));
		size_t limit = static_cast<size_t>(System::Environment::ReadAsUInt64("NANY_MEMORY_LIMIT"));
		if (0 == limit)
			nany_memalloc_set_default(&(cf->allocator));
		else
			nany_memalloc_set_with_limit(&(cf->allocator), limit);
		cf->build.entrypoint.size  = 4;
		cf->build.entrypoint.c_str = "main";
		cf->program.entrypoint = cf->build.entrypoint;
		// default output
		nyconsole_cf_set_stdcout(&(cf->console));
	}
}


extern "C" void nyrun_cf_release(nyrun_cf_t* cf) {
	if (cf) {
		if (cf->console.release)
			cf->console.release(&(cf->console));
		if (cf->allocator.release)
			cf->allocator.release(&(cf->allocator));
	}
}
