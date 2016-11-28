#pragma once
#include "libnanyc.h"


namespace ny { class Project; class IntrinsicTable; }

namespace ny {
namespace nsl {
namespace import {


//! Import 'std.core'
void core(Project&);

//! Import tests for 'std.core'
void unittests(Project&);

//! Import intrinsics related to string manipulation
void string(IntrinsicTable&);

//! Import 'std.memory'
void memory(IntrinsicTable&);

//! Import intrinsics related to IO accesses
void io(IntrinsicTable&);

//! Import intrinsics related to process manipulation
void process(IntrinsicTable&);

//! Import intrinsics related to environment variables manipulation
void env(IntrinsicTable&);

//! Import intrinsics related to console management
void console(IntrinsicTable&);

//! Import intrinsics related to digest
void digest(IntrinsicTable&);


} // namespace import
} // namespace nsl
} // namespace ny
