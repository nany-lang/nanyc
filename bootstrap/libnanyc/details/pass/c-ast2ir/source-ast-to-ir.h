#pragma once
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"

namespace ny::compiler {

bool passTransformASTToIR(ny::compiler::Source&, Logs::Report&, const nycompile_opts_t&);

} // ny::compiler
