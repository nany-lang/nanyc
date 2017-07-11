#include "details/intrinsic/std.h"
#include "details/intrinsic/catalog.h"
#include <yuni/core/string.h>
#include <yuni/core/system/environment.h>

using namespace Yuni;

static void nyinx_env_set(nyvmthread_t*, void* varname, void* content) {
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		AnyString cont;
		if (content)
			cont = *(reinterpret_cast<String*>(content));
		System::Environment::Set(name, cont);
	}
}

static void nyinx_env_unset(nyvmthread_t*, void* varname) {
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		System::Environment::Unset(name);
	}
}

static void* nyinx_env_read(nyvmthread_t* vm, void* varname, void* defvalue) {
	auto* string = vm_allocate<String>(vm);
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		bool read = System::Environment::Read(name, *string);
		if (not read and defvalue)
			*string = *(reinterpret_cast<String*>(defvalue));
	}
	else {
		if (defvalue)
			*string = *(reinterpret_cast<String*>(defvalue));
	}
	return string;
}

static bool nyinx_env_read_as_bool(nyvmthread_t*, void* varname, bool defvalue) {
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		return System::Environment::ReadAsBool(name, defvalue);
	}
	return defvalue;
}

static int64_t nyinx_env_read_as_i64(nyvmthread_t*, void* varname, int64_t defvalue) {
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		return System::Environment::ReadAsInt64(name, defvalue);
	}
	return defvalue;
}

static uint64_t nyinx_env_read_as_u64(nyvmthread_t*, void* varname, uint64_t defvalue) {
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		return System::Environment::ReadAsUInt64(name, defvalue);
	}
	return defvalue;
}

static bool nyinx_env_exists(nyvmthread_t*, void* varname) {
	if (varname) {
		AnyString name = *(reinterpret_cast<String*>(varname));
		return System::Environment::Exists(name);
	}
	return false;
}

namespace ny {
namespace intrinsic {
namespace import {

void env(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc_env_set",    nyinx_env_set);
	intrinsics.emplace("__nanyc_env_unset",  nyinx_env_unset);
	intrinsics.emplace("__nanyc_env_read",   nyinx_env_read);
	intrinsics.emplace("__nanyc_env_asbool", nyinx_env_read_as_bool);
	intrinsics.emplace("__nanyc_env_asi64",  nyinx_env_read_as_i64);
	intrinsics.emplace("__nanyc_env_asu64",  nyinx_env_read_as_u64);
	intrinsics.emplace("__nanyc_env_exists", nyinx_env_exists);
}

} // namespace import
} // namespace intrinsic
} // namespace ny
