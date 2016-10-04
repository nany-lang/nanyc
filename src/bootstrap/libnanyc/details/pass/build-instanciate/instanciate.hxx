#pragma once
#include "instanciate.h"



namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	inline void SequenceBuilder::pushNewFrame(Atom& atom)
	{
		auto* newframe = build.allocate<AtomStackFrame>(atom);
		newframe->previous = frame;
		frame = newframe;
	}

	inline void SequenceBuilder::popFrame()
	{
		auto* previous = frame->previous;
		build.deallocate(frame);
		frame = previous;
	}


	inline void SequenceBuilder::PushedParameters::clear()
	{
		func.indexed.clear();
		func.named.clear();
		gentypes.indexed.clear();
		gentypes.named.clear();
	}


	inline bool SequenceBuilder::canBeAcquired(const Classdef& cdef) const
	{
		bool success = not cdef.isBuiltinOrVoid();
		#ifndef NDEBUG
		if (unlikely(success and cdeftable.findClassdefAtom(cdef) == nullptr))
		{
			// do not generate any error if already reported
			if (frame->atomid != cdef.clid.atomid() or frame->verify(cdef.clid.lvid()))
				return (ice() << "canBeAcquired: invalid atom " << cdef.clid << ", " << cdef.print(cdeftable));
		}
		#endif
		return success;
	}


	inline bool SequenceBuilder::canBeAcquired(const CLID& clid) const
	{
		return canBeAcquired(cdeftable.classdef(clid));
	}


	inline bool SequenceBuilder::canBeAcquired(LVID lvid) const
	{
		assert(frame != nullptr);
		assert(lvid < frame->lvids.size());
		return canBeAcquired(CLID{frame->atomid, lvid});
	}


	inline bool SequenceBuilder::canGenerateCode() const
	{
		return (codeGenerationLock == 0);
	}


	inline bool SequenceBuilder::checkForIntrinsicParamCount(const AnyString& name, uint32_t count)
	{
		return (pushedparams.func.indexed.size() == count)
			or complainIntrinsicParameterCount(name, count);
	}


	inline void SequenceBuilder::acquireObject(LVID lvid)
	{
		assert(lvid > 1);
		assert(canBeAcquired(lvid));

		out->emitRef(lvid);

		// force unref
		assert(lvid < frame->lvids.size());
		frame->lvids[lvid].autorelease = true;
	}


	inline void SequenceBuilder::tryToAcquireObject(LVID lvid)
	{
		if (canBeAcquired(lvid))
			acquireObject(lvid);
	}


	template<class T> inline void SequenceBuilder::tryToAcquireObject(LVID lvid, const T& type)
	{
		if (canBeAcquired(type))
			acquireObject(lvid);
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
