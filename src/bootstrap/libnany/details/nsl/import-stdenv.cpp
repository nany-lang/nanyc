#include "import-stdcore.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/core/system/environment.h>

using namespace Yuni;




namespace Nany
{
namespace Builtin
{


	static void yn_env_set(nyvm_t*, void* varname, void* content)
	{
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			AnyString cont;
			if (content)
				cont = *(reinterpret_cast<String*>(content));
			System::Environment::Set(name, cont);
		}
	}


	static void yn_env_unset(nyvm_t*, void* varname)
	{
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			System::Environment::Unset(name);
		}
	}


	static void* yn_env_read(nyvm_t* vm, void* varname, void* defvalue)
	{
		auto* string = vm_allocate<String>(vm);
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			bool read = System::Environment::Read(name, *string);
			if (not read and defvalue)
				*string = *(reinterpret_cast<String*>(defvalue));
		}
		else
		{
			if (defvalue)
				*string = *(reinterpret_cast<String*>(defvalue));
		}
		return string;
	}


	static bool yn_env_read_as_bool(nyvm_t*, void* varname, bool defvalue)
	{
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			return System::Environment::ReadAsBool(name, defvalue);
		}
		return defvalue;
	}

	static int64_t yn_env_read_as_i64(nyvm_t*, void* varname, int64_t defvalue)
	{
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			return System::Environment::ReadAsInt64(name, defvalue);
		}
		return defvalue;
	}

	static uint64_t yn_env_read_as_u64(nyvm_t*, void* varname, uint64_t defvalue)
	{
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			return System::Environment::ReadAsUInt64(name, defvalue);
		}
		return defvalue;
	}

	static bool yn_env_exists(nyvm_t*, void* varname)
	{
		if (varname)
		{
			AnyString name = *(reinterpret_cast<String*>(varname));
			return System::Environment::Exists(name);
		}
		return false;
	}




} // namespace Builtin
} // namespace Nany



namespace Nany
{

	void importNSLEnv(IntrinsicTable& intrinsics)
	{
		intrinsics.add("yuni.env.set",    Builtin::yn_env_set);
		intrinsics.add("yuni.env.unset",  Builtin::yn_env_unset);
		intrinsics.add("yuni.env.read",   Builtin::yn_env_read);
		intrinsics.add("yuni.env.asbool", Builtin::yn_env_read_as_bool);
		intrinsics.add("yuni.env.asi64",  Builtin::yn_env_read_as_i64);
		intrinsics.add("yuni.env.asu64",  Builtin::yn_env_read_as_u64);
		intrinsics.add("yuni.env.exists", Builtin::yn_env_exists);
	}


} // namespace Nany
