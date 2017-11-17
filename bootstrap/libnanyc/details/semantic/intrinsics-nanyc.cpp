#include "semantic-analysis.h"
#include "deprecated-error.h"
#include "type-check.h"
#ifdef YUNI_OS_UNIX
#include <unistd.h>
#endif
#include "details/ir/emit.h"
#include "ref-unref.h"
#include <limits>

using namespace Yuni;

namespace ny::semantic::intrinsic {

namespace {

bool intrinsicReinterpret(Analyzer& seq, uint32_t lvid) {
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
	if (canBeAcquired(seq, cdef) and seq.canGenerateCode()) { // re-acquire the object
		ir::emit::ref(seq.out, lvid);
		lvidinfo.autorelease = true;
		lvidinfo.scope = seq.frame->scope;
	}
	return true;
}

bool intrinsicMemcheckerHold(Analyzer& seq, uint32_t lvid) {
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

template<class T> bool intrinsicNanycTypeSizeT(Analyzer& seq, uint32_t lvid) {
	static_assert(sizeof(T) >= sizeof(uint16_t) and sizeof(T) <= sizeof(uint64_t), "size mistmatch");
	ny::CType ctype;
	if (std::numeric_limits<T>::is_signed) {
		switch (sizeof(T)) {
			case sizeof(uint64_t): ctype = ny::CType::t_i64; break;
			case sizeof(uint32_t): ctype = ny::CType::t_i32; break;
			case sizeof(uint16_t): ctype = ny::CType::t_i16; break;
		}
	}
	else {
		switch (sizeof(T)) {
			case sizeof(uint64_t): ctype = ny::CType::t_u64; break;
			case sizeof(uint32_t): ctype = ny::CType::t_u32; break;
			case sizeof(uint16_t): ctype = ny::CType::t_u16; break;
		}
	}
	auto& libtype = seq.cdeftable.atoms().core.object[(uint32_t) ctype];
	seq.cdeftable.substitute(lvid).mutateToAtom(libtype.pointer());
	if (seq.canGenerateCode())
		ir::emit::constantu64(seq.out, lvid, 0);
	return true;
}

using BuiltinIntrinsic = bool (*)(Analyzer&, uint32_t);

static const std::unordered_map<AnyString, std::pair<uint32_t, BuiltinIntrinsic>> builtinDispatch = {
	{"__reinterpret",            { 2, &intrinsicReinterpret }},
	{"__nanyc_memchecker_hold",  { 2, &intrinsicMemcheckerHold }},
	{"__nanyc_type_size_t",      { 0, &intrinsicNanycTypeSizeT<size_t> }},
	{"__nanyc_type_ssize_t",     { 0, &intrinsicNanycTypeSizeT<ssize_t> }},
	{"__nanyc_type_uintptr_t",   { 0, &intrinsicNanycTypeSizeT<uintptr_t> }},
	{"__nanyc_type_intptr_t",    { 0, &intrinsicNanycTypeSizeT<intptr_t> }},
};

} // namespace

Tribool::Value nanycSpecifics(Analyzer& analyzer, const AnyString& name, uint32_t lvid, bool produceError) {
	assert(not name.empty());
	assert(analyzer.frame != nullptr);
	auto it = builtinDispatch.find(name);
	if (unlikely(it == builtinDispatch.end())) {
		if (produceError)
			complain::unknownIntrinsic(name);
		return Tribool::Value::indeterminate;
	}
	// checking for parameters
	uint32_t count = it->second.first;
	if (unlikely(not analyzer.checkForIntrinsicParamCount(name, count)))
		return Tribool::Value::no;
	analyzer.frame->lvids(lvid).synthetic = false;
	// intrinsic builtin found !
	return ((it->second.second))(analyzer, lvid) ? Tribool::Value::yes : Tribool::Value::no;
}

} // ny::semantic::intrinsic
