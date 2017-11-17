#pragma once
#include <yuni/core/string.h>

namespace ny::ir { struct Instruction; }
namespace ny::ir { struct Sequence; }

namespace ny::complain {

bool exception();
bool exception(const std::exception&);

bool invalidAtom(const char* what);
bool invalidAtomForFuncReturn(const AnyString& symbol);

bool invalidAtomMapping(const AnyString& atom);
bool invalidRecursiveAtom(const AnyString& atom);

bool inconsistentGenericTypeParameterIndex();

} // ny::complain
