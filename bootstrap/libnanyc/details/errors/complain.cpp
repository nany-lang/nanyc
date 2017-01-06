#include "complain.h"
#include "details/errors/errors.h"


namespace ny {
namespace complain {


bool exception() {
	return (ice() << "unknown exception");
}


bool exception(const std::exception& e) {
	return (ice() << "exception: " << e.what());
}


bool invalidAtom(const char* what) {
	ice() << "invalid atom: " << what;
	return false;
}


bool invalidAtomForFuncReturn(const AnyString& symbol) {
	ice() << "invalid atom pointer in func return type for '" << symbol << '\'';
	return false;
}


bool invalidAtomMapping(const AnyString& atom) {
	ice() << "failed to remap atom '" << atom << '\'';
	return false;
}


bool invalidRecursiveAtom(const AnyString& atom) {
	ice() << "cannot mark non function '" << atom << "' as recursive";
	return false;
}


bool inconsistentGenericTypeParameterIndex() {
	return (ice() << "generic type parameter index inconsistent with size()");
}


} // namespace complain
} // namespace ny
