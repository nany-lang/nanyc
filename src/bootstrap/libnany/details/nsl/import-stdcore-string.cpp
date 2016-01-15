#include "import-stdcore.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>

using namespace Yuni;




namespace Nany
{
namespace Builtin
{


	static void* yn_string_new(nytctx_t* tctx)
	{
		void* p = tctx->context->memory.allocate(tctx->context, sizeof(String));
		return new (p) String{};
	}

	static void yn_string_delete(nytctx_t* tctx, void* string)
	{
		(reinterpret_cast<String*>(string))->~String();
		tctx->context->memory.release(tctx->context, string, sizeof(String));
	}

	static uint64_t yn_string_size(nytctx_t*, void* string)
	{
		return (reinterpret_cast<String*>(string))->size();
	}

	static void yn_string_assign(nytctx_t*, void* string, void* rhs)
	{
		auto& other = *(reinterpret_cast<String*>(rhs));
		reinterpret_cast<String*>(string)->assign(other);
	}

	static void yn_string_clear(nytctx_t*, void* string)
	{
		reinterpret_cast<String*>(string)->clear();
	}




} // namespace Builtin
} // namespace Nany



namespace Nany
{

	void importNSLCoreString(IntrinsicTable& intrinsics)
	{
		intrinsics.add("yuni.string.new",    Builtin::yn_string_new);
		intrinsics.add("yuni.string.delete", Builtin::yn_string_delete);
		intrinsics.add("yuni.string.clear",  Builtin::yn_string_clear);
		intrinsics.add("yuni.string.size",   Builtin::yn_string_size);
		intrinsics.add("yuni.string.assign", Builtin::yn_string_assign);
	}


} // namespace Nany
