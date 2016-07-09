#include "import-stdcore.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>

using namespace Yuni;




namespace Nany
{
namespace Builtin
{


	static void* yn_string_new(nyvm_t* tctx)
	{
		return vm_allocate<String>(tctx);
	}

	static void yn_string_delete(nyvm_t* tctx, void* string)
	{
		vm_deallocate(tctx, reinterpret_cast<String*>(string));
	}

	static uint64_t yn_string_size(nyvm_t*, void* string)
	{
		return (reinterpret_cast<String*>(string))->size();
	}

	static void yn_string_append_str(nyvm_t*, void* string, void* rhs)
	{
		auto& other = *(reinterpret_cast<String*>(rhs));
		reinterpret_cast<String*>(string)->append(other);
	}

	static void yn_string_append_cstring(nyvm_t*, void* string, void* ptr, uint64_t size)
	{
		const char* const text = reinterpret_cast<const char* const>(ptr);
		reinterpret_cast<String*>(string)->append(text, static_cast<uint32_t>(size));
	}


	static void yn_string_clear(nyvm_t*, void* string)
	{
		reinterpret_cast<String*>(string)->clear();
	}

	static bool yn_string_is_equal(nyvm_t*, void* string, void* rhs)
	{
		auto& other = *(reinterpret_cast<String*>(rhs));
		return reinterpret_cast<String*>(string)->equals(other);
	}


	static void yn_string_cout(nyvm_t* tctx, void* string)
	{
		auto& str = *(reinterpret_cast<String*>(string));
		vm_print(tctx, str);
	}


	template<class T> struct IntCast { typedef T value; };
	template<> struct IntCast<int8_t> { typedef int32_t value; };
	template<> struct IntCast<uint8_t> { typedef uint32_t value; };
	template<> struct IntCast<int16_t> { typedef int32_t value; };
	template<> struct IntCast<uint16_t> { typedef uint32_t value; };

	template<class T> static void yn_string_append(nyvm_t*, void* string, T value)
	{
		reinterpret_cast<String*>(string)->append(static_cast<typename IntCast<T>::value>(value));
	}

	static void yn_string_append_ptr(nyvm_t*, void* string, void* ptr)
	{
		reinterpret_cast<String*>(string)->append(ptr);
	}



} // namespace Builtin
} // namespace Nany



namespace Nany
{

	void importNSLCoreString(IntrinsicTable& intrinsics)
	{
		intrinsics.add("yuni.string.new",            Builtin::yn_string_new);
		intrinsics.add("yuni.string.delete",         Builtin::yn_string_delete);
		intrinsics.add("yuni.string.clear",          Builtin::yn_string_clear);
		intrinsics.add("yuni.string.size",           Builtin::yn_string_size);
		intrinsics.add("yuni.string.append.string",  Builtin::yn_string_append_str);
		intrinsics.add("yuni.string.append.cstring", Builtin::yn_string_append_cstring);

		intrinsics.add("yuni.string.append.u8",   Builtin::yn_string_append<uint8_t>);
		intrinsics.add("yuni.string.append.u16",  Builtin::yn_string_append<uint16_t>);
		intrinsics.add("yuni.string.append.u32",  Builtin::yn_string_append<uint32_t>);
		intrinsics.add("yuni.string.append.u64",  Builtin::yn_string_append<uint64_t>);
		intrinsics.add("yuni.string.append.i8",   Builtin::yn_string_append<int8_t>);
		intrinsics.add("yuni.string.append.i16",  Builtin::yn_string_append<int16_t>);
		intrinsics.add("yuni.string.append.i32",  Builtin::yn_string_append<int32_t>);
		intrinsics.add("yuni.string.append.i64",  Builtin::yn_string_append<int64_t>);
		intrinsics.add("yuni.string.append.f32",  Builtin::yn_string_append<float>);
		intrinsics.add("yuni.string.append.f64",  Builtin::yn_string_append<double>);
		intrinsics.add("yuni.string.append.ptr",  Builtin::yn_string_append_ptr);
		intrinsics.add("yuni.string.equals",      Builtin::yn_string_is_equal);

		intrinsics.add("yuni.string.cout", Builtin::yn_string_cout);
	}


} // namespace Nany
