#pragma once
#include <yuni/core/string.h>
#include <exception>


namespace ny {
namespace complain {


bool exception();
bool exception(const std::exception&);

bool invalidAtom(const char* what);
bool invalidAtomForFuncReturn(const AnyString& symbol);

bool invalidAtomMapping(const AnyString& atom);
bool invalidRecursiveAtom(const AnyString& atom);

bool inconsistentGenericTypeParameterIndex();


} // namespace complain
} // namespace ny
