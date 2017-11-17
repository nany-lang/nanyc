#pragma once
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"

namespace ny::compiler {

using UsesCallback = void (*)(void*, const AnyString&);

bool passDuplicateAndNormalizeAST(ny::compiler::Source&, Logs::Report&, UsesCallback, void* userdata);

} // ny::compiler
