#pragma once
#include "libnanyc.h"
#include "details/intrinsic/catalog.h"


namespace ny { class Project; }

namespace ny {
namespace nsl {
namespace import {


//! Import 'std.core'
void core(Project&);

//! Import tests for 'std.core'
void unittests(Project&);

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


} // namespace import
} // namespace nsl
} // namespace ny
