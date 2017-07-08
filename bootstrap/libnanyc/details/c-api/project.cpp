#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/project.h"

using namespace Yuni;


extern "C" void nyproject_cf_init(nyproject_cf_t* cf) {
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nyproject_cf_t));
}


extern "C" nyproject_t* nyproject_create(const nyproject_cf_t* cf) {
	bool withUnittests = (cf->with_nsl_unittests != nyfalse);
	std::unique_ptr<ny::Project> project;
	try {
		if (cf) {
			project = std::make_unique<ny::Project>(*cf, withUnittests);
		}
		else {
			nyproject_cf_t ncf;
			nyproject_cf_init(&ncf);
			project = std::make_unique<ny::Project>(ncf, withUnittests);
		}
		// making sure that user-events do not destroy the project by mistake
		project->addRef();
		return project.release()->self();
	}
	catch (...) {}
	return nullptr;
}


extern "C" void nyproject_ref(nyproject_t* project) {
	if (project)
		ny::ref(project).addRef();
}


extern "C" void nyproject_unref(nyproject_t* ptr) {
	if (ptr) {
		auto& project = ny::ref(ptr);
		if (project.release())
			delete &project;
	}
}


extern "C" nybool_t nyproject_add_source_from_file_n(nyproject_t* ptr, const char* filename, size_t len) {
	if (ptr and filename and len != 0 and len < 32 * 1024) {
		AnyString path{filename, static_cast<uint32_t>(len)};
		ny::ref(ptr).targets.anonym->addSourceFromFile(path);
		return nytrue;
	}
	return nyfalse;
}


extern "C" nybool_t nyproject_add_source_from_file(nyproject_t* ptr, const char* filename) {
	if (filename != nullptr) {
		size_t length = strlen(filename);
		return nyproject_add_source_from_file_n(ptr, filename, length);
	}
	return nyfalse;
}


extern "C" nybool_t nyproject_add_source_n(nyproject_t* ptr, const char* text, size_t len) {
	if (ptr and text and len != 0 and len < 512 * 1024 * 1024) { // arbitrary
		AnyString source{text, static_cast<uint32_t>(len)};
		ny::ref(ptr).targets.anonym->addSource("<unknown>", source);
		return nytrue;
	}
	return nyfalse;
}


extern "C" nybool_t nyproject_add_source(nyproject_t* ptr, const char* text) {
	if (text != nullptr) {
		size_t length = strlen(text);
		return nyproject_add_source_n(ptr, text, length);
	}
	return nyfalse;
}


extern "C" void nyproject_lock(const nyproject_t* ptr) {
	if (ptr)
		ny::ref(ptr).mutex.lock();
}


extern "C" void nyproject_unlock(const nyproject_t* ptr) {
	if (ptr)
		ny::ref(ptr).mutex.unlock();
}


extern "C" nybool_t nyproject_trylock(const nyproject_t* ptr) {
	return ((ptr) ? ny::ref(ptr).mutex.trylock() : false) ? nytrue : nyfalse;
}
