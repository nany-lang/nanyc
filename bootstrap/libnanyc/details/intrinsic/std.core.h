#pragma once
#include "libnanyc.h"
#include "details/intrinsic/catalog.h"

namespace ny { class Project; }

namespace ny {
namespace nsl {
namespace import {

//! Import intrinsics related to string manipulation
void string(ny::intrinsic::Catalog&);

//! Import 'std.memory'
void memory(ny::intrinsic::Catalog&);

//! Import intrinsics related to IO accesses
void io(ny::intrinsic::Catalog&);

//! Import intrinsics related to process manipulation
void process(ny::intrinsic::Catalog&);

//! Import intrinsics related to environment variables manipulation
void env(ny::intrinsic::Catalog&);

//! Import intrinsics related to console management
void console(ny::intrinsic::Catalog&);

//! Import intrinsics related to digest
void digest(ny::intrinsic::Catalog&);

inline void all(intrinsic::Catalog& intrinsics) {
	ny::nsl::import::string(intrinsics);
	ny::nsl::import::process(intrinsics);
	ny::nsl::import::env(intrinsics);
	ny::nsl::import::io(intrinsics);
	ny::nsl::import::memory(intrinsics);
	ny::nsl::import::console(intrinsics);
	ny::nsl::import::digest(intrinsics);
}

} // namespace import
} // namespace nsl
} // namespace ny
