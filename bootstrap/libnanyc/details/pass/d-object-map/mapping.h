#pragma once
#include "libnanyc.h"
#include <yuni/thread/mutex.h>
#include <vector>
#include <utility>
#include "details/atom/atom.h"
#include "details/errors/errors.h"
#include "details/ir/isa/opcodes.h"
#include "details/ir/instruction.h"


namespace ny::Pass {


struct MappingOptions final {
	//! Flag to evaluate the whole sequence, or only a portion of it
	bool evaluateWholeSequence = true;
	//! Prefix to prepend for the first atom created by the mapping
	AnyString prefixNameForFirstAtomCreated;
	//! IR Code offset
	uint32_t offset = 0;
	//! Does the first atom created own the sequence
	bool firstAtomOwnSequence = false;

	//! The first atom created by the mapping
	// This value might be used when a mapping is done on the fly
	// (while instanciating code for example)
	Atom* firstAtomCreated = nullptr;
};


bool map(Atom& parent, ClassdefTable&, Yuni::Mutex&, ir::Sequence&, MappingOptions&);


} // namespace ny::Pass
