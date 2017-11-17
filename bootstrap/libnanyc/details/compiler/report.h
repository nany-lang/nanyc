#pragma once
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"

namespace ny::compiler::report {

void raisedErrorsForAllAtoms(ny::compiler::Compdb&, ny::Logs::Report&);

} // namespace ny::compiler::report
