#pragma once

namespace ny::semantic {

struct Analyzer;

//! Generate the function to initialize the class variables to their default values
void produceMemberVarDefaultInitialization(Analyzer&);

//! Generate the destructor of the current class
void produceMemberVarDefaultDispose(Analyzer&);

//! Generate the clone function of the current class
void produceMemberVarDefaultClone(Analyzer&);

} // ny::semantic
