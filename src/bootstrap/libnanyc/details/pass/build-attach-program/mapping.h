#pragma once
#include "../../fwd.h"
#include <yuni/thread/mutex.h>
#include <vector>
#include <utility>
#include "details/atom/atom.h"
#include "details/errors/errors.h"
#include "details/ir/isa/opcodes.h"
#include "details/ir/instruction.h"




namespace ny
{
namespace Pass
{
namespace Mapping
{


	class SequenceMapping final
	{
	public:
		SequenceMapping(ClassdefTable& cdeftable, Yuni::Mutex& mutex, IR::Sequence& sequence);

		bool map(Atom& parentAtom, uint32_t offset = 0);


	public:
		//! The classdef table (must be protected by 'mutex' in some passes)
		ClassdefTable& cdeftable;
		//! Mutex for the cdeftable
		Yuni::Mutex& mutex;
		//! Current sequence
		IR::Sequence& currentSequence;
		//! Flag to evaluate the whole sequence, or only a portion of it
		bool evaluateWholeSequence = true;
		//! Prefix to prepend for the first atom created by the mapping
		AnyString prefixNameForFirstAtomCreated;
		//! The first atom created by the mapping
		// This value might be used when a mapping is done on the fly
		// (while instanciating code for example)
		Atom* firstAtomCreated = nullptr;

	}; // class SequenceMapping




} // namespace Mapping
} // namespace Pass
} // namespace ny
