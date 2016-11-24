#include "instanciate.h"
#include "instanciate-error.h"
#include "type-check.h"
#ifdef YUNI_OS_UNIX
#include <unistd.h>
#endif
#include "details/ir/emit.h"
#include "reflection.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


namespace {


bool intrinsicReinterpret(SequenceBuilder& seq, uint32_t lvid) {
	assert(seq.pushedparams.func.indexed.size() == 2);
	uint32_t lhs    = seq.pushedparams.func.indexed[0].lvid;
	uint32_t tolvid = seq.pushedparams.func.indexed[1].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, tolvid});
	// copy the type, without any check
	auto& spare = seq.cdeftable.substitute(lvid);
	spare.import(cdef);
	spare.qualifiers = cdef.qualifiers;
	if (seq.canGenerateCode())
		ir::emit::copy(seq.out, lvid, lhs);
	auto& lvidinfo = seq.frame->lvids(lvid);
	lvidinfo.synthetic = false;
	if (seq.canBeAcquired(cdef) and seq.canGenerateCode()) { // re-acquire the object
		ir::emit::ref(seq.out, lvid);
		lvidinfo.autorelease = true;
		lvidinfo.scope = seq.frame->scope;
	}
	return true;
}


bool intrinsicMemcheckerHold(SequenceBuilder& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdefptr = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (not cdefptr.isRawPointer())
		return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefptr, "'__pointer'");
	uint32_t sizelvid = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, sizelvid});
	if (not cdefsize.isBuiltinU64())
		return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefsize, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::hold(seq.out, ptrlvid, sizelvid);
	return true;
}


using BuiltinIntrinsic = bool (*)(SequenceBuilder&, uint32_t);

static const std::unordered_map<AnyString, std::pair<uint32_t, BuiltinIntrinsic>> builtinDispatch = {
	{"__reinterpret",               { 2,  &intrinsicReinterpret, }},
	{"__nanyc_memchecker_hold",     { 2,  &intrinsicMemcheckerHold, }},
	// reflect
	{"__nanyc_reflect_name",        { 1, &reflect::name }},
	{"__nanyc_reflect_classname",   { 1, &reflect::classname }},
	{"__nanyc_reflect_keyword",     { 1, &reflect::keyword }},
	{"__nanyc_reflect_filename",    { 1, &reflect::filename }},
	{"__nanyc_reflect_column",      { 1, &reflect::column }},
	{"__nanyc_reflect_line",        { 1, &reflect::line }},
	{"__nanyc_reflect_is_class",    { 1, &reflect::isClass }},
	{"__nanyc_reflect_is_func",     { 1, &reflect::isFunc }},
	{"__nanyc_reflect_is_var",      { 1, &reflect::isVar }},
	{"__nanyc_reflect_is_typedef",  { 1, &reflect::isTypedef }},
	{"__nanyc_reflect_is_view",     { 1, &reflect::isView }},
	{"__nanyc_reflect_is_operator", { 1, &reflect::isOperator }},
	{"__nanyc_reflect_is_ctor",     { 1, &reflect::ctor }},
	{"__nanyc_reflect_is_dtor",     { 1, &reflect::dtor }},
	{"__nanyc_reflect_callable",    { 1, &reflect::callable }},
	{"__nanyc_reflect_anonymous",   { 1, &reflect::anonymous }},
	{"__nanyc_reflect_sizeof",      { 1, &reflect::bytes }},
	{"__nanyc_reflect_begin",       { 1, &reflect::begin }},
	{"__nanyc_reflect_item",        { 0, &reflect::item }},
	{"__nanyc_reflect_end",         { 0, &reflect::end }},
	{"__nanyc_reflect_props_count", { 1, &reflect::props::count }},
	{"__nanyc_reflect_vars_count",  { 1, &reflect::funcs::count }},
	{"__nanyc_reflect_funcs_count", { 1, &reflect::vars::count }},
};


} // anonymous namespace


Tribool::Value SequenceBuilder::instanciateBuiltinIntrinsicSpecific(const AnyString& name, uint32_t lvid,
		bool canProduceError) {
	assert(not name.empty());
	assert(frame != nullptr);
	auto it = builtinDispatch.find(name);
	if (unlikely(it == builtinDispatch.end())) {
		if (canProduceError)
			complain::unknownIntrinsic(name);
		return Tribool::Value::indeterminate;
	}
	// checking for parameters
	uint32_t count = it->second.first;
	if (unlikely(not checkForIntrinsicParamCount(name, count)))
		return Tribool::Value::no;
	frame->lvids(lvid).synthetic = false;
	// intrinsic builtin found !
	return ((it->second.second))(*this, lvid) ? Tribool::Value::yes : Tribool::Value::no;
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
