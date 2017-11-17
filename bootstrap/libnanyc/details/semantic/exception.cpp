#include "exception.h"

using namespace yuni;

namespace ny::semantic {

InvalidAtom::InvalidAtom(const char* context, const Atom* atom, const Classdef* cdef) {
	if (context)
		msg << context << ": ";
	if (atom)
		msg << "invalid atom '" << atom->caption() << '\'';
	else
		msg << "invalid null atom";
	if (debugmode and cdef)
		msg << " (from " << cdef->clid << ')';
}

} // ny::semantic
