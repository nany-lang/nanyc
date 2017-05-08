#pragma once
#include "semantic-analysis.h"
#include "details/ir/emit.h"


namespace ny {
namespace semantic {


inline void Analyzer::pushNewFrame(Atom& atom) {
	auto* newframe = build.allocate<AtomStackFrame>(atom, frame);
	frame = newframe;
}


inline void Analyzer::popFrame() {
	auto* previous = frame->previous;
	build.deallocate(frame);
	frame = previous;
}


inline bool Analyzer::canGenerateCode() const {
	return (codeGenerationLock == 0);
}


inline bool Analyzer::checkForIntrinsicParamCount(const AnyString& name, uint32_t count) {
	return (pushedparams.func.indexed.size() == count)
		or complainIntrinsicParameterCount(name, count);
}


inline void Analyzer::releaseAllScopedVariables() {
	releaseScopedVariables(0 /*all scopes*/);
}


} // namespace semantic
} // namespace ny
