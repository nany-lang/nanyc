#pragma once
#include "libnanyc.h"
#include "details/intrinsic/catalog.h"

namespace ny { class Project; }

namespace ny::intrinsic::import {

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
	ny::intrinsic::import::string(intrinsics);
	ny::intrinsic::import::process(intrinsics);
	ny::intrinsic::import::env(intrinsics);
	ny::intrinsic::import::io(intrinsics);
	ny::intrinsic::import::memory(intrinsics);
	ny::intrinsic::import::console(intrinsics);
	ny::intrinsic::import::digest(intrinsics);
}

} // ny::intrinsic::import
