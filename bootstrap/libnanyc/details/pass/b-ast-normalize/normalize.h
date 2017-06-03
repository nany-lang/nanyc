#pragma once
#include "details/compiler/compiler.h"
#include "details/reporting/report.h"

namespace ny {
namespace compiler {

bool passDuplicateAndNormalizeAST(ny::compiler::Source&, Logs::Report&);

} // namespace compiler
} // namespace ny
