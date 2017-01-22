#pragma once
#include "semantic-analysis.h"
#include "details/errors/complain.h"


namespace ny {
namespace semantic {


struct InvalidAtom: public ny::complain::ICE {
	InvalidAtom(const char* context, const Atom*, const Classdef*);
};


} // namespace semantic
} // namespace ny
