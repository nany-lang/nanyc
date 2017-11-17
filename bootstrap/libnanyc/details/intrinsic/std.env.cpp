#include "details/intrinsic/std.h"
#include "details/intrinsic/catalog.h"
#include "details/intrinsic/std.internals.utils.h"
#include <yuni/core/string.h>
#include <yuni/core/system/environment.h>

using namespace Yuni;

static void nyinx_env_set(nyvmthread_t*, const char* name, uint32_t nlen, const char* val, uint32_t vlen) {
	if (nlen != 0) {
		ny::intrinsic::FromNanycString varname(name, nlen);
		ny::intrinsic::FromNanycString value(val, vlen);
		System::Environment::Set(varname.anystring(), value.anystring());
	}
}

static void nyinx_env_unset(nyvmthread_t*, const char* name, uint32_t nlen) {
	if (nlen != 0) {
		ny::intrinsic::FromNanycString varname(name, nlen);
		System::Environment::Unset(varname.anystring());
	}
}

static void* nyinx_env_read(nyvmthread_t* vm, const char* name, uint32_t nlen, const char* dval, uint32_t dlen) {
	if (nlen != 0) {
		ny::intrinsic::FromNanycString varname(name, nlen);
		yuni::String value;
		bool read = System::Environment::Read(varname.anystring(), value);
		if (read)
			return ny::intrinsic::makeInterimNanycString(vm, value);
	}
	if (dlen == 0)
		return nullptr;
	ny::intrinsic::FromNanycString defvalue(dval, dlen);
	return ny::intrinsic::makeInterimNanycString(vm, defvalue);
}

static int64_t nyinx_env_read_as_i64(nyvmthread_t*, const char* name, uint32_t nlen, int64_t defvalue) {
	if (nlen != 0) {
		ny::intrinsic::FromNanycString varname(name, nlen);
		return System::Environment::ReadAsInt64(varname.anystring(), defvalue);
	}
	return defvalue;
}

static uint64_t nyinx_env_read_as_u64(nyvmthread_t*, const char* name, uint32_t nlen, uint64_t defvalue) {
	if (nlen != 0) {
		ny::intrinsic::FromNanycString varname(name, nlen);
		return System::Environment::ReadAsUInt64(varname.anystring(), defvalue);
	}
	return defvalue;
}

static bool nyinx_env_exists(nyvmthread_t*, const char* name, uint32_t nlen) {
	if (nlen != 0) {
		ny::intrinsic::FromNanycString varname(name, nlen);
		return System::Environment::Exists(varname.anystring());
	}
	return false;
}

namespace ny::intrinsic::import {

void env(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc_env_set",    nyinx_env_set);
	intrinsics.emplace("__nanyc_env_unset",  nyinx_env_unset);
	intrinsics.emplace("__nanyc_env_read",   nyinx_env_read);
	intrinsics.emplace("__nanyc_env_asi64",  nyinx_env_read_as_i64);
	intrinsics.emplace("__nanyc_env_asu64",  nyinx_env_read_as_u64);
	intrinsics.emplace("__nanyc_env_exists", nyinx_env_exists);
}

} // ny::intrinsic::import
