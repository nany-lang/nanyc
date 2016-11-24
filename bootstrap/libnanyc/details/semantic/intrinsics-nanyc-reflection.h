#pragma once
#include "semantic-analysis.h"


namespace ny {
namespace semantic {
namespace reflect {


bool name(Analyzer&, uint32_t lvid);

bool classname(Analyzer&, uint32_t lvid);

bool keyword(Analyzer&, uint32_t lvid);

bool filename(Analyzer&, uint32_t lvid);

bool column(Analyzer&, uint32_t lvid);

bool line(Analyzer&, uint32_t lvid);

bool isClass(Analyzer&, uint32_t lvid);

bool isFunc(Analyzer&, uint32_t lvid);

bool isVar(Analyzer&, uint32_t lvid);

bool isTypedef(Analyzer&, uint32_t lvid);

bool isView(Analyzer&, uint32_t lvid);

bool isOperator(Analyzer&, uint32_t lvid);

bool ctor(Analyzer&, uint32_t lvid);

bool dtor(Analyzer&, uint32_t lvid);

bool callable(Analyzer&, uint32_t lvid);

bool anonymous(Analyzer&, uint32_t lvid);

bool bytes(Analyzer&, uint32_t lvid);

bool foreach(Analyzer&, uint32_t lvid);


namespace props {


bool count(Analyzer&, uint32_t lvid);


} // namespace props


namespace vars {


bool count(Analyzer&, uint32_t lvid);


} // namespace vars


namespace funcs {


bool count(Analyzer&, uint32_t lvid);


} // namespace funcs


} // namespace reflect
} // namespace semantic
} // namespace Nany
