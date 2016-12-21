#pragma once
#include "instanciate.h"
#include "details/ir/emit.h"
#include <cassert>


namespace ny {
namespace Pass {
namespace Instanciate {
namespace {


inline bool canBeAcquired(const SequenceBuilder& sb, const Classdef& cdef) {
	bool success = not cdef.isBuiltinOrVoid();
	if (yuni::debugmode) {
		if (unlikely(success and sb.cdeftable.findClassdefAtom(cdef) == nullptr)) {
			if (sb.frame->atomid != cdef.clid.atomid() or sb.frame->verify(cdef.clid.lvid()))
				return (ice() << "canBeAcquired: invalid atom " << cdef.clid << ", " << cdef.print(sb.cdeftable));
		}
	}
	return success;
}


inline bool canBeAcquired(const SequenceBuilder& sb, const CLID& clid) {
	return canBeAcquired(sb, sb.cdeftable.classdef(clid));
}


inline bool canBeAcquired(const SequenceBuilder& sb, uint32_t lvid) {
	assert(sb.frame != nullptr);
	return canBeAcquired(sb, CLID{sb.frame->atomid, lvid});
}


inline void acquireObject(SequenceBuilder& sb, uint32_t lvid) {
	assert(lvid > 1 and "can not acquire the returned value");
	assert(canBeAcquired(sb, lvid));
	assert(sb.frame != nullptr);
	ir::emit::ref(sb.out, lvid);
	sb.frame->lvids(lvid).autorelease = true;
}


inline void tryToAcquireObject(SequenceBuilder& sb, uint32_t lvid) {
	if (canBeAcquired(sb, lvid))
		acquireObject(sb, lvid);
}


template<class T>
inline void tryToAcquireObject(SequenceBuilder& sb, uint32_t lvid, const T& type) {
	if (canBeAcquired(sb, type))
		acquireObject(sb, lvid);
}


inline void tryUnrefObject(SequenceBuilder& sb, uint32_t lvid) {
	auto* frame = sb.frame;
	if (unlikely(not frame->verify(lvid)))
		return;
	if (unlikely(frame->lvids(lvid).synthetic)) {
		auto err = (error() << "cannot unref a synthetic object");
		if (yuni::debugmode)
			err << " {" << frame->atomid << ',' << lvid << '}';
		return;
	}
	auto& cdef  = sb.cdeftable.classdefFollowClassMember(CLID{frame->atomid, lvid});
	if (canBeAcquired(sb, cdef)) {
		auto* atom = sb.cdeftable.findClassdefAtom(cdef);
		if (unlikely(nullptr == atom)) {
			ice() << "invalid atom for 'unref' opcode";
			return frame->invalidate(lvid);
		}
		if (0 == atom->classinfo.dtor.atomid) {
			if (unlikely(not sb.instanciateAtomClassDestructor(*atom, lvid)))
				return frame->invalidate(lvid);
			assert(atom->classinfo.dtor.atomid != 0);
		}
		if (sb.canGenerateCode())
			ir::emit::unref(sb.out, lvid, atom->classinfo.dtor.atomid, atom->classinfo.dtor.instanceid);
	}
}


} // namespace
} // namespace Instanciate
} // namespace Pass
} // namespace ny
