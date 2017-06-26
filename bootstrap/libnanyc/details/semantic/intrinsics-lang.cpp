#include "semantic-analysis.h"
#include "intrinsics.h"
#include "deprecated-error.h"
#include "type-check.h"
#ifdef YUNI_OS_UNIX
#include <unistd.h>
#endif
#include "details/ir/emit.h"
#include "ref-unref.h"

using namespace Yuni;




namespace ny {
namespace semantic {
namespace intrinsic {

namespace {


bool intrinsicOSIsUnix(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode())
		ir::emit::constantbool(seq.out, lvid, yuni::System::unix);
	return true;
}


bool intrinsicOSIsPosix(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode()) {
		#if defined(YUNI_OS_UNIX) && defined(_POSIX_VERSION)
		ir::emit::constantbool(seq.out, lvid, true);
		#else
		ir::emit::constantbool(seq.out, lvid, false);
		#endif
	}
	return true;
}


bool intrinsicOSIsLinux(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode())
		ir::emit::constantbool(seq.out, lvid, yuni::System::linux);
	return true;
}


bool intrinsicOSIsAIX(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode()) {
		#ifdef YUNI_OS_AIX
		ir::emit::constantbool(seq.out, lvid, true);
		#else
		ir::emit::constantbool(seq.out, lvid, false);
		#endif
	}
	return true;
}


bool intrinsicOSIsWindows(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode())
		ir::emit::constantbool(seq.out, lvid, yuni::System::windows);
	return true;
}


bool intrinsicOSIsCygwin(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode()) {
		#if defined(__CYGWIN32__) || defined(__CYGWIN__)
		ir::emit::constantbool(seq.out, lvid, true);
		#else
		ir::emit::constantbool(seq.out, lvid, false);
		#endif
	}
	return true;
}


bool intrinsicOSIsMacOS(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode())
		ir::emit::constantbool(seq.out, lvid, yuni::System::macos);
	return true;
}


bool intrinsicOSIsBSD(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_bool);
	if (seq.canGenerateCode()) {
		#if defined(YUNI_OS_MACOS) || defined(YUNI_OS_OPENBSD) || defined(YUNI_OS_FREEBSD) \
			|| defined(YUNI_OS_NETBSD) || defined(YUNI_OS_DRAGONFLY)
		ir::emit::constantbool(seq.out, lvid, true);
		#else
		ir::emit::constantbool(seq.out, lvid, false);
		#endif
	}
	return true;
}


bool intrinsicFieldset(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t lvidsid = seq.pushedparams.func.indexed[1].lvid;
	uint32_t sid = seq.frame->lvids(lvidsid).text_sid;
	if (unlikely(sid == (uint32_t) - 1))
		return (ice() << "invalid string-id for field name (got lvid " << lvidsid << ')');
	AnyString varname = seq.out->stringrefs[sid];
	if (unlikely(varname.empty()))
		return (ice() << "invalid empty field name");
	Atom* parent = seq.frame->atom.isClass() ? &seq.frame->atom : seq.frame->atom.parent;
	if (unlikely(!parent))
		return (ice() << "invalid parent atom for __nanyc_fieldset");
	Atom* varatom = nullptr;
	parent->eachChild(varname, [&](Atom & child) -> bool {
		if (unlikely(varatom))
			return (ice() << "duplicate variable member '" << varname << "'");
		varatom = &child;
		return true;
	});
	if (unlikely(!varatom))
		return (ice() << "invalid variable member atom for __nanyc_fieldset");
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdefvar = seq.cdeftable.classdefFollowClassMember(varatom->returnType.clid);
	auto& cdef    = seq.cdeftable.classdef(CLID{seq.frame->atomid, objlvid});
	// flag for implicitly converting objects (bool, i32...) into builtin (__bool, __i32...)
	bool implicitBuiltin = cdefvar.isBuiltin() and (not cdef.isBuiltinOrVoid());
	if (implicitBuiltin) {
		// checking if an implicit can be performed (if rhs is a 'builtin' type)
		auto* atomrhs = (seq.cdeftable.findClassdefAtom(cdef));
		implicitBuiltin = (seq.cdeftable.atoms().core.object[(uint32_t) cdefvar.kind] == atomrhs);
	}
	if (not implicitBuiltin) {
		auto similarity = TypeCheck::isSimilarTo(seq, cdef, cdefvar);
		if (unlikely(TypeCheck::Match::none == similarity))
			return seq.complainInvalidType("fieldset: ", cdef, cdefvar);
	}
	if (seq.canGenerateCode()) {
		if (not implicitBuiltin) {
			tryToAcquireObject(seq, objlvid);
			ir::emit::fieldset(seq.out, objlvid, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
		}
		else {
			uint32_t lvidvalue = seq.createLocalVariables();
			ir::emit::fieldget(seq.out, lvidvalue, objlvid, 0);
			ir::emit::fieldset(seq.out, lvidvalue, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
		}
	}
	return true;
}


bool intrinsicRef(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	if (seq.canGenerateCode())
		tryToAcquireObject(seq, seq.pushedparams.func.indexed[0].lvid);
	return true;
}


bool intrinsicUnref(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	if (seq.canGenerateCode())
		tryUnrefObject(seq, seq.pushedparams.func.indexed[0].lvid);
	return true;
}


bool intrinsicPointer(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_ptr);
	if (seq.canGenerateCode()) {
		uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
		if (canBeAcquired(seq, objlvid)) {
			// retrieving the pointer address of the object in input
			// the objlvid representing the object is already the address
			ir::emit::copy(seq.out, lvid, objlvid);
		}
		else {
			// can not be acquired, not an object - NULL
			ir::emit::constantu64(seq.out, lvid, 0);
		}
	}
	return true;
}


bool intrinsicSizeof(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_u64);
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	bool isBuiltinOrVoid = cdef.isBuiltinOrVoid();
	Atom* atom = nullptr;
	if (not isBuiltinOrVoid) {
		atom = seq.cdeftable.findClassdefAtom(cdef);
		if (unlikely(nullptr == atom))
			return (ice() << "invalid atom for sizeof operator");
	}
	if (seq.canGenerateCode()) {
		uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
		auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
		if (not isBuiltinOrVoid) {
			assert(atom != nullptr);
			ir::emit::type::objectSizeof(seq.out, lvid, atom->atomid);
		}
		else {
			uint64_t size = ny::ctypeSizeof(cdef.kind);
			ir::emit::constantu64(seq.out, lvid, size);
		}
	}
	return true;
}


bool intrinsicMemalloc(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_ptr);
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (not cdef.isBuiltinU64())
		return seq.complainIntrinsicParameter("memory.allocate", 0, cdef, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::allocate(seq.out, lvid, objlvid);
	return true;
}


bool intrinsicMemrealloc(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_ptr);
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdefptr = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (not cdefptr.isRawPointer())
		return seq.complainIntrinsicParameter("memory.realloc", 0, cdefptr, "'__pointer'");
	uint32_t oldsizelvid = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefOldsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, oldsizelvid});
	if (not cdefOldsize.isBuiltinU64())
		return seq.complainIntrinsicParameter("memory.realloc", 0, cdefOldsize, "'__u64'");
	uint32_t newsizelvid = seq.pushedparams.func.indexed[2].lvid;
	auto& cdefNewsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, newsizelvid});
	if (not cdefNewsize.isBuiltinU64())
		return seq.complainIntrinsicParameter("memory.realloc", 0, cdefNewsize, "'__u64'");
	if (seq.canGenerateCode()) {
		ir::emit::copy(seq.out, lvid, ptrlvid);
		ir::emit::memory::reallocate(seq.out, lvid, oldsizelvid, newsizelvid);
	}
	return true;
}


bool intrinsicMemFree(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (cdef.kind != CType::t_ptr)
		return seq.complainIntrinsicParameter("memory.dispose", 0, cdef, "'__u64'");
	uint32_t size = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
	if (unlikely(not cdefsize.isBuiltingUnsigned()))
		return seq.complainIntrinsicParameter("memory.dispose", 1, cdef, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::dispose(seq.out, objlvid, size);
	return true;
}


bool intrinsicMemfill(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (cdef.kind != CType::t_ptr)
		return seq.complainIntrinsicParameter("memory.memset", 0, cdef, "'__pointer'");
	uint32_t size = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
	if (unlikely(not cdefsize.isBuiltingUnsigned()))
		return seq.complainIntrinsicParameter("memory.memset", 1, cdef, "'__u64'");
	uint32_t patternlvid = seq.pushedparams.func.indexed[2].lvid;
	auto& cdefpattern = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, patternlvid});
	if (unlikely(not cdefpattern.isBuiltingUnsigned()))
		return seq.complainIntrinsicParameter("memory.memset", 2, cdef, "'__u32'");
	if (seq.canGenerateCode())
		ir::emit::memory::fill(seq.out, objlvid, size, patternlvid);
	return true;
}


bool intrinsicMemCopy(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("memory.copy", 0, cdef, "'__pointer'");
	uint32_t src = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsrc = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, src});
	if (unlikely(not cdefsrc.isRawPointer()))
		return seq.complainIntrinsicParameter("memory.copy", 1, cdef, "'__pointer'");
	uint32_t size = seq.pushedparams.func.indexed[2].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
	if (unlikely(not cdefsize.isBuiltinU64()))
		return seq.complainIntrinsicParameter("memory.copy", 2, cdef, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::copyNoOverlap(seq.out, objlvid, src, size);
	return true;
}


bool intrinsicMemMove(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("memory.move", 0, cdef, "'__pointer'");
	uint32_t src = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsrc = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, src});
	if (unlikely(not cdefsrc.isRawPointer()))
		return seq.complainIntrinsicParameter("memory.move", 1, cdef, "'__pointer'");
	uint32_t size = seq.pushedparams.func.indexed[2].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
	if (unlikely(not cdefsize.isBuiltinU64()))
		return seq.complainIntrinsicParameter("memory.move", 2, cdef, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::copy(seq.out, objlvid, src, size);
	return true;
}


bool intrinsicMemCmp(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_u64);
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("memory.cmp", 0, cdef, "'__pointer'");
	uint32_t src = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefsrc = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, src});
	if (unlikely(not cdefsrc.isRawPointer()))
		return seq.complainIntrinsicParameter("memory.cmp", 1, cdef, "'__pointer'");
	uint32_t size = seq.pushedparams.func.indexed[2].lvid;
	auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
	if (unlikely(not cdefsize.isBuiltinU64()))
		return seq.complainIntrinsicParameter("memory.cmp", 2, cdef, "'__u64'");
	if (seq.canGenerateCode()) {
		ir::emit::copy(seq.out, lvid, size);
		ir::emit::memory::compare(seq.out, objlvid, src, lvid);
	}
	return true;
}


bool intrinsicMemGetU64(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_u64);
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("load.u64", 0, cdef, "'__pointer'");
	if (seq.canGenerateCode())
		ir::emit::memory::loadu64(seq.out, lvid, ptrlvid);
	return true;
}


bool intrinsicMemGetU32(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_u32);
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("load.u32", 0, cdef, "'__pointer'");
	if (seq.canGenerateCode())
		ir::emit::memory::loadu32(seq.out, lvid, ptrlvid);
	return true;
}


bool intrinsicMemGetU8(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_u8);
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("load.u8", 0, cdef, "'__pointer'");
	if (seq.canGenerateCode())
		ir::emit::memory::loadu8(seq.out, lvid, ptrlvid);
	return true;
}


bool intrinsicMemGetPTR(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(CType::t_ptr);
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("load.ptr", 0, cdef, "'__pointer'");
	if (seq.canGenerateCode()) {
		if (sizeof(uint64_t) == sizeof(void*))
			ir::emit::memory::loadu64(seq.out, lvid, ptrlvid);
		else
			ir::emit::memory::loadu32(seq.out, lvid, ptrlvid);
	}
	return true;
}


bool intrinsicMemSetU64(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("store.u64", 0, cdef, "'__pointer'");
	uint32_t value = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
	if (unlikely(not cdefvalue.isBuiltinU64()))
		return seq.complainIntrinsicParameter("store.u64", 1, cdefvalue, "'__u64'");
	if (seq.canGenerateCode())
		ir::emit::memory::storeu64(seq.out, value, ptrlvid);
	return true;
}


bool intrinsicMemSetU32(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("store.u32", 0, cdef, "'__pointer'");
	uint32_t value = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
	if (unlikely(not cdefvalue.isBuiltinU32()))
		return seq.complainIntrinsicParameter("store.u32", 1, cdefvalue, "'__u32'");
	if (seq.canGenerateCode())
		ir::emit::memory::storeu32(seq.out, value, ptrlvid);
	return true;
}


bool intrinsicMemSetU8(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("store.u8", 0, cdef, "'__pointer'");
	uint32_t value = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
	if (unlikely(not cdefvalue.isBuiltinU8()))
		return seq.complainIntrinsicParameter("store.u8", 1, cdefvalue, "'__u8'");
	if (seq.canGenerateCode())
		ir::emit::memory::storeu8(seq.out, value, ptrlvid);
	return true;
}


bool intrinsicMemSetPTR(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
	if (unlikely(not cdef.isRawPointer()))
		return seq.complainIntrinsicParameter("storeptr", 0, cdef, "'__pointer'");
	uint32_t value = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
	if (unlikely(not cdefvalue.isRawPointer()))
		return seq.complainIntrinsicParameter("storeptr", 1, cdefvalue, "'__pointer'");
	if (seq.canGenerateCode()) {
		if (sizeof(uint64_t) == sizeof(void*))
			ir::emit::memory::storeu64(seq.out, value, ptrlvid);
		else
			ir::emit::memory::storeu32(seq.out, value, ptrlvid);
	}
	return true;
}


template<class T>
bool intrinsicStrlen(Analyzer& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToBuiltin(sizeof(T) == sizeof(uint32_t) ? CType::t_u32 : CType::t_u64);
	uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
	if (!cdef.isRawPointer())
		return seq.complainIntrinsicParameter("strlen", 0, cdef, "'__ptr'");
	if (seq.canGenerateCode()) {
		uint32_t bits = (sizeof(T) == sizeof(uint32_t) ? 32 : 64);
		ir::emit::memory::cstrlen(seq.out, lvid, bits, objlvid);
	}
	return true;
}


bool intrinsicNOT(Analyzer& seq, uint32_t lvid) {
	assert(seq.pushedparams.func.indexed.size() == 1);
	uint32_t lhs  = seq.pushedparams.func.indexed[0].lvid;
	auto& cdeflhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, lhs});
	Atom* atomBuiltinCast = nullptr;
	CType builtinlhs = cdeflhs.kind;
	if (builtinlhs == CType::t_any) { // implicit access to the internal 'pod' variable
		auto* atom = seq.cdeftable.findClassdefAtom(cdeflhs);
		if (atom != nullptr and atom->builtinMapping != CType::t_void) {
			atomBuiltinCast = atom;
			builtinlhs = atom->builtinMapping;
			uint32_t newlvid = seq.createLocalVariables();
			if (seq.canGenerateCode()) {
				ir::emit::trace(seq.out, "reading inner 'pod' variable");
				ir::emit::fieldget(seq.out, newlvid, lhs, 0);
			}
			lhs = newlvid;
			if (builtinlhs != CType::t_bool) // allow only bool for complex types
				builtinlhs = CType::t_any;
		}
	}
	if (unlikely(builtinlhs != CType::t_bool and builtinlhs != CType::t_ptr))
		return seq.complainIntrinsicParameter("not", 0, cdeflhs);
	// --- result of the operator
	if (atomBuiltinCast != nullptr) {
		// implicit convertion from builtin __bool to object bool
		atomBuiltinCast = Ref<Atom>::WeakPointer(seq.cdeftable.atoms().core.object[(uint32_t) CType::t_bool]);
		assert(atomBuiltinCast != nullptr);
		assert(not atomBuiltinCast->hasGenericParameters());
		Atom* remapAtom = seq.instanciateAtomClass(*atomBuiltinCast);
		if (unlikely(nullptr == remapAtom))
			return false;
		auto& opc = seq.cdeftable.substitute(lvid);
		opc.mutateToAtom(atomBuiltinCast);
		opc.qualifiers.ref = true;
		if (seq.canGenerateCode()) {
			// creating two variables on the stack
			uint32_t opresult   = seq.createLocalVariables(2);
			uint32_t sizeoflvid = opresult + 1;
			// RESULT: opresult: the first one is the result of the operation (and, +, -...)
			seq.cdeftable.substitute(opresult).mutateToBuiltin(CType::t_bool);
			ir::emit::opnot(seq.out, opresult, lhs);
			// SIZEOF: the second variable on the stack is `sizeof(<object>)`
			// (sizeof the object to allocate)
			seq.cdeftable.substitute(sizeoflvid).mutateToBuiltin(CType::t_u64);
			ir::emit::type::objectSizeof(seq.out, sizeoflvid, atomBuiltinCast->atomid);
			// ALLOC: memory allocation of the new temporary object
			ir::emit::memory::allocate(seq.out, lvid, sizeoflvid);
			ir::emit::ref(seq.out, lvid);
			seq.frame->lvids(lvid).autorelease = true;
			// reset the internal value of the object
			ir::emit::fieldset(seq.out, opresult, /*self*/lvid, 0); // builtin
		}
	}
	else {
		// no convertion to perform, direct call
		seq.cdeftable.substitute(lvid).mutateToBuiltin(builtinlhs);
		if (seq.canGenerateCode())
			ir::emit::opnot(seq.out, lvid, lhs);
	}
	return true;
}


bool intrinsicAssert(Analyzer& seq, uint32_t lvid) {
	assert(seq.pushedparams.func.indexed.size() == 1);
	// no return value
	seq.cdeftable.substitute(lvid).mutateToVoid();
	uint32_t lhs  = seq.pushedparams.func.indexed[0].lvid;
	auto& cdeflhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, lhs});
	Atom* atomBuiltinCast = nullptr;
	CType builtinlhs = cdeflhs.kind;
	if (builtinlhs == CType::t_any) { // implicit access to the internal 'pod' variable
		auto* atom = seq.cdeftable.findClassdefAtom(cdeflhs);
		if (atom != nullptr and atom->builtinMapping != CType::t_void) {
			if (debugmode and seq.canGenerateCode())
				ir::emit::trace(seq.out, "reading inner 'pod' variable");
			atomBuiltinCast = atom;
			builtinlhs = atom->builtinMapping;
			if (seq.canGenerateCode()) {
				uint32_t newlvid = seq.createLocalVariables();
				ir::emit::fieldget(seq.out, newlvid, lhs, 0);
				lhs = newlvid;
			}
		}
	}
	if (unlikely(builtinlhs != CType::t_bool))
		return seq.complainIntrinsicParameter("assert", 0, cdeflhs);
	// --- result of the operator
	if (atomBuiltinCast != nullptr) {
		// implicit convertion from builtin __bool to object bool
		atomBuiltinCast = Ref<Atom>::WeakPointer(seq.cdeftable.atoms().core.object[(uint32_t) CType::t_bool]);
		assert(atomBuiltinCast != nullptr);
		assert(not atomBuiltinCast->hasGenericParameters());
		Atom* remapAtom = seq.instanciateAtomClass(*atomBuiltinCast);
		if (unlikely(nullptr == remapAtom))
			return false;
	}
	if (seq.canGenerateCode())
		ir::emit::cassert(seq.out, lhs);
	return true;
}


constexpr static const CType promotion[ctypeCount][ctypeCount] = {
	/*void*/ {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*any*/  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*ptr*/  {CType::t_void, CType::t_void, CType::t_ptr, CType::t_void, CType::t_ptr, CType::t_ptr, CType::t_ptr, CType::t_ptr, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*bool*/ {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*u8 */  {CType::t_void, CType::t_void, CType::t_ptr, CType::t_void, CType::t_u8,  CType::t_u16, CType::t_u32, CType::t_u64, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*u16*/  {CType::t_void, CType::t_void, CType::t_ptr, CType::t_void, CType::t_u16, CType::t_u16, CType::t_u32, CType::t_u64, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*u32*/  {CType::t_void, CType::t_void, CType::t_ptr, CType::t_void, CType::t_u32, CType::t_u32, CType::t_u32, CType::t_u64, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*u64*/  {CType::t_void, CType::t_void, CType::t_ptr, CType::t_void, CType::t_u64, CType::t_u64, CType::t_u64, CType::t_u64, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void,},
	/*i8 */  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_i8,  CType::t_i16, CType::t_i32, CType::t_i64, CType::t_void, CType::t_void,},
	/*i16*/  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_i16, CType::t_i16, CType::t_i32, CType::t_i64, CType::t_void, CType::t_void,},
	/*i32*/  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_i32, CType::t_i32, CType::t_i32, CType::t_i64, CType::t_void, CType::t_void,},
	/*i64*/  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_i64, CType::t_i64, CType::t_i64, CType::t_i64, CType::t_void, CType::t_void,},
	/*f32*/  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_f32, CType::t_f32, CType::t_f32, CType::t_void, CType::t_f32, CType::t_f32, CType::t_f32, CType::t_void, CType::t_f32, CType::t_f64, },
	/*f64*/  {CType::t_void, CType::t_void, CType::t_void, CType::t_void, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, CType::t_f64, },
};


template<CType R, bool AcceptBool, bool AcceptInt, bool AcceptFloat,
		 void (* M)(ir::emit::IRCodeRef, uint32_t, uint32_t, uint32_t)>
inline bool emitBuiltinOperator(Analyzer& seq, uint32_t lvid, const char* const name) {
	assert(seq.pushedparams.func.indexed.size() == 2);
	// -- LHS - PARAMETER 0 --
	uint32_t lhs  = seq.pushedparams.func.indexed[0].lvid;
	auto& cdeflhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, lhs});
	Atom* atomBuiltinCast = nullptr;
	CType builtinlhs = cdeflhs.kind;
	if (builtinlhs == CType::t_any) { // implicit access to the internal 'pod' variable
		auto* atom = seq.cdeftable.findClassdefAtom(cdeflhs);
		if (atom != nullptr and atom->builtinMapping != CType::t_void) {
			builtinlhs = atom->builtinMapping;
			if (seq.canGenerateCode()) {
				uint32_t newlvid = seq.createLocalVariables();
				ir::emit::trace(seq.out, "reading inner 'pod' variable");
				ir::emit::fieldget(seq.out, newlvid, lhs, 0);
				lhs = newlvid;
			}
			else
				lhs = (uint32_t) - 1;
			atomBuiltinCast = atom;
		}
	}
	// -- RHS - PARAMETER 1 --
	uint32_t rhs  = seq.pushedparams.func.indexed[1].lvid;
	auto& cdefrhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, rhs});
	CType builtinrhs = cdefrhs.kind;
	if (builtinrhs == CType::t_any) { // implicit access to the internal 'pod' variable
		auto* atom = seq.cdeftable.findClassdefAtom(cdefrhs);
		if (atom != nullptr and (atom->builtinMapping != CType::t_void)) {
			builtinrhs = atom->builtinMapping;
			if (seq.canGenerateCode()) {
				uint32_t newlvid = seq.createLocalVariables();
				ir::emit::trace(seq.out, "reading inner 'pod' variable");
				ir::emit::fieldget(seq.out, newlvid, rhs, 0);
				rhs = newlvid;
			}
			else
				rhs = (uint32_t) - 1;
			atomBuiltinCast = atom;
		}
	}
	// -- implicit type promotion --
	if (builtinlhs != builtinrhs) {
		builtinlhs = promotion[(uint32_t) builtinlhs][(uint32_t) builtinrhs];
		if (unlikely(builtinlhs == CType::t_void)) {
			seq.complainIntrinsicParameter(name, 0, cdeflhs);
			return seq.complainIntrinsicParameter(name, 1, cdefrhs);
		}
		if (atomBuiltinCast != nullptr and builtinlhs != CType::t_ptr)
			atomBuiltinCast = Ref<Atom>::WeakPointer(seq.cdeftable.atoms().core.object[(uint32_t) builtinlhs]);
	}
	switch (builtinlhs) {
		case CType::t_bool: {
			if (not AcceptBool)
				return seq.complainBuiltinIntrinsicDoesNotAccept(name, "booleans");
			break;
		}
		case CType::t_i8:
		case CType::t_i16:
		case CType::t_i32:
		case CType::t_i64:
		case CType::t_u8:
		case CType::t_u16:
		case CType::t_u32:
		case CType::t_u64:
		case CType::t_ptr: {
			if (not AcceptInt)
				return seq.complainBuiltinIntrinsicDoesNotAccept(name, "integer literals");
			break;
		}
		case CType::t_f32:
		case CType::t_f64: {
			if (not AcceptFloat)
				return seq.complainBuiltinIntrinsicDoesNotAccept(name, "floating-point numbers");
			break;
		}
		case CType::t_void:
		case CType::t_any: {
			return seq.complainIntrinsicParameter(name, 0, cdeflhs);
		}
	}
	// --- result of the operator
	CType rettype = (R == CType::t_any) ? builtinlhs : R;
	if (rettype != CType::t_ptr and atomBuiltinCast != nullptr
		and seq.shortcircuit.label == 0) { // the result is a real instance
		// implicit convertion from builtin (__i32...) to object (i32...)
		if (R != CType::t_any) { // force the result type
			atomBuiltinCast = Ref<Atom>::WeakPointer(seq.cdeftable.atoms().core.object[(uint32_t) R]);
			assert(atomBuiltinCast != nullptr);
			assert(not atomBuiltinCast->hasGenericParameters());
		}
		Atom* remapAtom = seq.instanciateAtomClass(*atomBuiltinCast);
		if (unlikely(nullptr == remapAtom))
			return false;
		auto& opc = seq.cdeftable.substitute(lvid);
		opc.mutateToAtom(atomBuiltinCast);
		opc.qualifiers.ref = true;
		if (seq.canGenerateCode()) {
			ir::emit::trace(seq.out, [&]() {
				return String("builtin ") << name;
			});
			// creating two variables on the stack
			uint32_t opresult   = seq.createLocalVariables(2);
			uint32_t sizeoflvid = opresult + 1;
			// RESULT: opresult: the first one is the result of the operation (and, +, -...)
			seq.cdeftable.substitute(opresult).mutateToBuiltin(rettype);
			M(seq.out, opresult, lhs, rhs);
			// SIZEOF: the second variable on the stack is `sizeof(<object>)`
			// (sizeof the object to allocate)
			seq.cdeftable.substitute(sizeoflvid).mutateToBuiltin(CType::t_u64);
			ir::emit::type::objectSizeof(seq.out, sizeoflvid, atomBuiltinCast->atomid);
			// ALLOC: memory allocation of the new temporary object
			ir::emit::memory::allocate(seq.out, lvid, sizeoflvid);
			ir::emit::ref(seq.out, lvid);
			seq.frame->lvids(lvid).autorelease = true;
			// reset the internal value of the object
			ir::emit::fieldset(seq.out, opresult, /*self*/lvid, 0); // builtin
		}
	}
	else {
		// no convertion to perform, direct call
		seq.cdeftable.substitute(lvid).mutateToBuiltin(rettype);
		if (seq.canGenerateCode()) {
			ir::emit::trace(seq.out, [&]() {
				return String("builtin ") << name;
			});
			M(seq.out, lvid, lhs, rhs);
		}
	}
	return true;
}


bool intrinsicAND(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 1, 1, 0, &ir::emit::opand>(seq, lvid, "and");
}

bool intrinsicOR(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 1, 1, 0, &ir::emit::opor>(seq, lvid, "or");
}

bool intrinsicXOR(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 1, 1, 0, &ir::emit::opxor>(seq, lvid, "xor");
}

bool intrinsicMOD(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 1, 1, 0, &ir::emit::opmod>(seq, lvid, "mod");
}

bool intrinsicADD(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 1, 0, &ir::emit::opadd>(seq, lvid, "add");
}

bool intrinsicSUB(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 1, 0, &ir::emit::opsub>(seq, lvid, "sub");
}

bool intrinsicDIV(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 1, 0, &ir::emit::opdiv>(seq, lvid, "div");
}

bool intrinsicMUL(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 1, 0, &ir::emit::opmul>(seq, lvid, "mul");
}

bool intrinsicIDIV(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 1, 0, &ir::emit::opidiv>(seq, lvid, "idiv");
}

bool intrinsicIMUL(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 1, 0, &ir::emit::opimul>(seq, lvid, "imul");
}

bool intrinsicFADD(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 0, 1, &ir::emit::opfadd>(seq, lvid, "fadd");
}

bool intrinsicFSUB(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opigte>(seq, lvid, "igte");
	return emitBuiltinOperator<CType::t_any, 0, 0, 1, &ir::emit::opfsub>(seq, lvid, "fsub");
}

bool intrinsicFDIV(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 0, 1, &ir::emit::opfdiv>(seq, lvid, "fdiv");
}

bool intrinsicFMUL(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_any, 0, 0, 1, &ir::emit::opfmul>(seq, lvid, "fmul");
}

bool intrinsicEQ(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opeq>(seq, lvid, "eq");
}

bool intrinsicNEQ(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opneq>(seq, lvid, "neq");
}

bool intrinsicFLT(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opflt>(seq, lvid, "flt");
}

bool intrinsicFLTE(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opflte>(seq, lvid, "flte");
}

bool intrinsicFGT(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opfgt>(seq, lvid, "fgt");
}

bool intrinsicFGTE(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opfgte>(seq, lvid, "fgte");
}

bool intrinsicLT(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::oplt>(seq, lvid, "lt");
}

bool intrinsicLTE(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::oplte>(seq, lvid, "lte");
}

bool intrinsicILT(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opilt>(seq, lvid, "ilt");
}

bool intrinsicILTE(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opilte>(seq, lvid, "ilte");
}

bool intrinsicGT(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opgt>(seq, lvid, "gt");
}

bool intrinsicGTE(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opgte>(seq, lvid, "gte");
}

bool intrinsicIGT(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opigt>(seq, lvid, "igt");
}

bool intrinsicIGTE(Analyzer& seq, uint32_t lvid) {
	return emitBuiltinOperator<CType::t_bool, 1, 1, 1, &ir::emit::opigte>(seq, lvid, "igte");
}


using BuiltinIntrinsic = bool (*)(Analyzer&, uint32_t);

static const std::unordered_map<AnyString, std::pair<uint32_t, BuiltinIntrinsic>> builtinDispatch = {
	//
	{"^fieldset",       { 2,  &intrinsicFieldset,    }},
	//
	{"memory.allocate", { 1,  &intrinsicMemalloc,    }},
	{"memory.realloc",  { 3,  &intrinsicMemrealloc,  }},
	{"memory.dispose",  { 2,  &intrinsicMemFree,     }},
	{"memory.fill",     { 3,  &intrinsicMemfill,     }},
	{"memory.copy",     { 3,  &intrinsicMemCopy,     }},
	{"memory.move",     { 3,  &intrinsicMemMove,     }},
	{"memory.cmp",      { 3,  &intrinsicMemCmp,      }},
	{"load.ptr",        { 1,  &intrinsicMemGetPTR,   }},
	{"load.u64",        { 1,  &intrinsicMemGetU64,   }},
	{"load.u32",        { 1,  &intrinsicMemGetU32,   }},
	{"load.u8",         { 1,  &intrinsicMemGetU8,    }},
	{"store.ptr",       { 2,  &intrinsicMemSetPTR,   }},
	{"store.u64",       { 2,  &intrinsicMemSetU64,   }},
	{"store.u32",       { 2,  &intrinsicMemSetU32,   }},
	{"store.u8",        { 2,  &intrinsicMemSetU8,    }},
	//
	{"ref",             { 1,  &intrinsicRef,         }},
	{"unref",           { 1,  &intrinsicUnref,       }},
	{"sizeof",          { 1,  &intrinsicSizeof,      }},
	{"pointer",         { 1,  &intrinsicPointer,     }},
	//
	{"and",             { 2,  &intrinsicAND,         }},
	{"or",              { 2,  &intrinsicOR,          }},
	{"xor",             { 2,  &intrinsicXOR,         }},
	{"mod",             { 2,  &intrinsicMOD,         }},
	//
	{"not",             { 1,  &intrinsicNOT,         }},
	//
	{"add",             { 2,  &intrinsicADD,         }},
	{"sub",             { 2,  &intrinsicSUB,         }},
	{"div",             { 2,  &intrinsicDIV,         }},
	{"mul",             { 2,  &intrinsicMUL,         }},
	{"idiv",            { 2,  &intrinsicIDIV,        }},
	{"imul",            { 2,  &intrinsicIMUL,        }},
	{"fadd",            { 2,  &intrinsicFADD,        }},
	{"fsub",            { 2,  &intrinsicFSUB,        }},
	{"fdiv",            { 2,  &intrinsicFDIV,        }},
	{"fmul",            { 2,  &intrinsicFMUL,        }},
	//
	{"eq",              { 2,  &intrinsicEQ,          }},
	{"neq",             { 2,  &intrinsicNEQ,         }},
	{"flt",             { 2,  &intrinsicFLT,         }},
	{"flte",            { 2,  &intrinsicFLTE,        }},
	{"fgt",             { 2,  &intrinsicFGT,         }},
	{"fgte",            { 2,  &intrinsicFGTE,        }},
	{"lt",              { 2,  &intrinsicLT,          }},
	{"lte",             { 2,  &intrinsicLTE,         }},
	{"ilt",             { 2,  &intrinsicILT,         }},
	{"ilte",            { 2,  &intrinsicILTE,        }},
	{"gt",              { 2,  &intrinsicGT,          }},
	{"gte",             { 2,  &intrinsicGTE,         }},
	{"igt",             { 2,  &intrinsicIGT,         }},
	{"igte",            { 2,  &intrinsicIGTE,        }},
	//
	{"assert",          { 1,  &intrinsicAssert,      }},

	{"strlen32",        { 1,  &intrinsicStrlen<uint32_t> }},
	{"strlen64",        { 1,  &intrinsicStrlen<uint64_t> }},

	{"os.is.linux",     { 0,  &intrinsicOSIsLinux      }},
	{"os.is.unix",      { 0,  &intrinsicOSIsUnix       }},
	{"os.is.posix",     { 0,  &intrinsicOSIsPosix      }},
	{"os.is.macos",     { 0,  &intrinsicOSIsMacOS      }},
	{"os.is.bsd",       { 0,  &intrinsicOSIsBSD        }},
	{"os.is.aix",       { 0,  &intrinsicOSIsAIX        }},
	{"os.is.windows",   { 0,  &intrinsicOSIsWindows    }},
	{"os.is.cygwin",    { 0,  &intrinsicOSIsCygwin     }},
};


} // anonymous namespace


Tribool::Value langOrNanycSpecifics(Analyzer& analyzer, const AnyString& name, uint32_t lvid, bool produceError) {
	assert(not name.empty());
	assert(analyzer.frame != nullptr);
	auto& frame = *analyzer.frame;
	if (produceError) {
		auto& pushedparams = analyzer.pushedparams;
		// named parameters are not accepted
		if (unlikely(not pushedparams.func.named.empty())) {
			analyzer.complainIntrinsicWithNamedParameters(name);
			return Tribool::Value::no;
		}
		// generic type parameters are not accepted
		if (unlikely(not pushedparams.gentypes.indexed.empty() or not pushedparams.gentypes.named.empty())) {
			analyzer.complainIntrinsicWithGenTypeParameters(name);
			return Tribool::Value::no;
		}
		// checking if one parameter was already flag as 'error'
		for (uint32_t i = 0u; i != pushedparams.func.indexed.size(); ++i) {
			if (unlikely(not frame.verify(pushedparams.func.indexed[i].lvid)))
				return Tribool::Value::no;
		}
	}
	if (name.first() == '_')
		return intrinsic::nanycSpecifics(analyzer, name, lvid, produceError);
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
	frame.lvids(lvid).synthetic = false;
	// intrinsic builtin found !
	return ((it->second.second))(analyzer, lvid) ? Tribool::Value::yes : Tribool::Value::no;
}


} // namespace intrinsic
} // namespace semantic
} // namespace ny
