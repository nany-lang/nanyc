#pragma once
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"

namespace ny {
namespace compiler {

bool passTransformASTToIR(ny::compiler::Source&, Logs::Report&, const nycompile_opts_t&);

} // namespace compiler
} // namespace ny
