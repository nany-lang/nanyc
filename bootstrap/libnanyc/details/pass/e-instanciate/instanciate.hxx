#pragma once
#include "instanciate.h"
#include "details/ir/emit.h"


namespace ny {
namespace Pass {
namespace Instanciate {


inline void SequenceBuilder::pushNewFrame(Atom& atom) {
	auto* newframe = build.allocate<AtomStackFrame>(atom, frame);
	frame = newframe;
}


inline void SequenceBuilder::popFrame() {
	auto* previous = frame->previous;
	build.deallocate(frame);
	frame = previous;
}


inline bool SequenceBuilder::canGenerateCode() const {
	return (codeGenerationLock == 0);
}


inline bool SequenceBuilder::checkForIntrinsicParamCount(const AnyString& name, uint32_t count) {
	return (pushedparams.func.indexed.size() == count)
		or complainIntrinsicParameterCount(name, count);
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
