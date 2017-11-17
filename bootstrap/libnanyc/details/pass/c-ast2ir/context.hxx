#pragma once
#include "context.h"

namespace ny::ir::Producer {

inline void Context::invalidateLastDebugLine() {
	m_previousDbgLine = (uint32_t) - 1; // forcing debug infos
}

} // namespace ny::ir::Producer
