#pragma once
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"

namespace ny {
namespace compiler {
namespace report {

void raisedErrorsForAllAtoms(ny::compiler::Compdb&, ny::Logs::Report&);

} // namespace report
} // namespace compiler
} // namespace ny
