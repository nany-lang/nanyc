#include "runtime.h"
#include "details/intrinsic/intrinsic-table.h"

using namespace Yuni;





static void* nanyc_string_new(nyvm_t* tctx)
{
	return vm_allocate<String>(tctx);
}

static void nanyc_string_delete(nyvm_t* tctx, void* string)
{
	vm_deallocate(tctx, reinterpret_cast<String*>(string));
}

static uint64_t nanyc_string_size(nyvm_t*, void* string)
{
	return (reinterpret_cast<String*>(string))->size();
}

static void nanyc_string_append_str(nyvm_t*, void* string, void* rhs)
{
	auto& other = *(reinterpret_cast<String*>(rhs));
	reinterpret_cast<String*>(string)->append(other);
}

static void nanyc_string_append_cstring(nyvm_t*, void* string, void* ptr, uint64_t size)
{
	const char* const text = reinterpret_cast<const char* const>(ptr);
	reinterpret_cast<String*>(string)->append(text, static_cast<uint32_t>(size));
}


static void nanyc_string_clear(nyvm_t*, void* string)
{
	reinterpret_cast<String*>(string)->clear();
}

static bool nanyc_string_is_equal(nyvm_t*, void* string, void* rhs)
{
	auto& other = *(reinterpret_cast<String*>(rhs));
	return reinterpret_cast<String*>(string)->equals(other);
}


static void nanyc_string_cout(nyvm_t* tctx, void* string)
{
	auto& str = *(reinterpret_cast<String*>(string));
	vm_print(tctx, str);
}


template<class T> struct IntCast { typedef T value; };
template<> struct IntCast<int8_t> { typedef int32_t value; };
template<> struct IntCast<uint8_t> { typedef uint32_t value; };
template<> struct IntCast<int16_t> { typedef int32_t value; };
template<> struct IntCast<uint16_t> { typedef uint32_t value; };

template<class T> static void nanyc_string_append(nyvm_t*, void* string, T value)
{
	reinterpret_cast<String*>(string)->append(static_cast<typename IntCast<T>::value>(value));
}

static void nanyc_string_append_ptr(nyvm_t*, void* string, void* ptr)
{
	reinterpret_cast<String*>(string)->append(ptr);
}



namespace Nany
{

	void importNSLCoreString(IntrinsicTable& intrinsics)
	{
		intrinsics.add("nanyc.string.new",            nanyc_string_new);
		intrinsics.add("nanyc.string.delete",         nanyc_string_delete);
		intrinsics.add("nanyc.string.clear",          nanyc_string_clear);
		intrinsics.add("nanyc.string.size",           nanyc_string_size);
		intrinsics.add("nanyc.string.append.string",  nanyc_string_append_str);
		intrinsics.add("nanyc.string.append.cstring", nanyc_string_append_cstring);

		intrinsics.add("nanyc.string.append.u8",   nanyc_string_append<uint8_t>);
		intrinsics.add("nanyc.string.append.u16",  nanyc_string_append<uint16_t>);
		intrinsics.add("nanyc.string.append.u32",  nanyc_string_append<uint32_t>);
		intrinsics.add("nanyc.string.append.u64",  nanyc_string_append<uint64_t>);
		intrinsics.add("nanyc.string.append.i8",   nanyc_string_append<int8_t>);
		intrinsics.add("nanyc.string.append.i16",  nanyc_string_append<int16_t>);
		intrinsics.add("nanyc.string.append.i32",  nanyc_string_append<int32_t>);
		intrinsics.add("nanyc.string.append.i64",  nanyc_string_append<int64_t>);
		intrinsics.add("nanyc.string.append.f32",  nanyc_string_append<float>);
		intrinsics.add("nanyc.string.append.f64",  nanyc_string_append<double>);
		intrinsics.add("nanyc.string.append.ptr",  nanyc_string_append_ptr);
		intrinsics.add("nanyc.string.equals",      nanyc_string_is_equal);

		intrinsics.add("nanyc.string.cout", nanyc_string_cout);
	}

} // namespace Nany
