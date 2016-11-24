#pragma once
#include "instanciate.h"


namespace ny {
namespace reflect {


bool name(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool classname(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool keyword(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool filename(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool column(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool line(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool isClass(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool isFunc(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool isVar(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool isTypedef(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool isView(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool isOperator(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool ctor(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool dtor(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool callable(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool anonymous(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool bytes(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool begin(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool item(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);

bool end(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);


namespace props {


bool count(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);


} // namespace props


namespace vars {


bool count(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);


} // namespace vars


namespace funcs {


bool count(ny::Pass::Instanciate::SequenceBuilder&, uint32_t lvid);


} // namespace funcs


} // namespace reflect
} // namespace Nany
