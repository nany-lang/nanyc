#pragma once
#include "libnanyc.h"
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"
#include "details/atom/atom.h"
#include "func-overload-match.h"

namespace ny::semantic {

struct Analyzer;

struct Settings final {
	Settings(Atom& atom, ClassdefTableView& cdeftable,
			ny::compiler::Compdb& compdb,
			decltype(FuncOverloadMatch::result.params)& params,
			decltype(FuncOverloadMatch::result.params)& tmplparams)
		: cdeftable(cdeftable)
		, atom(atom)
		, compdb(compdb)
		, params(params)
		, tmplparams(tmplparams)
		, report(std::make_unique<Logs::Message>(Logs::Level::none)) {
		returnType.mutateToAny();
	}

	//! The original view to the classdef table
	ClassdefTableView& cdeftable;
	//! The atom to instanciate
	std::reference_wrapper<Atom> atom;
	//! The parent atom, if any
	Atom* parentAtom = nullptr;
	//! Instance
	uint32_t instanceid = (uint32_t) - 1;
	ny::compiler::Compdb& compdb;
	//! Parameters used for instanciation
	decltype(FuncOverloadMatch::result.params)& params;
	//! Template parameters
	decltype(FuncOverloadMatch::result.params)& tmplparams;
	//!
	Classdef returnType;
	//! Flag to determine whether the code can be generated or not
	bool canGenerateCode = true;
	//! Flag to determine whether the code generate errors or not
	bool canGenerateErrors = true;
	//! Make the layer persistent
	bool shouldMergeLayer = false;
	//! Parent
	Analyzer* parent = nullptr;
	//! Error reporting
	std::unique_ptr<Logs::Message> report;
	bool signatureOnly = false;

}; // struct Settings

/*!
** \brief Instanciate atom
*/
bool instanciateAtom(Settings& info);

bool instanciateAtomParameterTypes(Settings& info);

/*!
** \brief Post-processing for resetting types on 'stackalloc' opcodes
**
** Those opcodes may not have the good declared type (most likely something like 'any')
** (always update even if sometimes not necessary, easier for debugging)
*/
void updateTypesInAllStackallocOp(ir::Sequence&, ClassdefTableView&, uint32_t atomid);

/*!
** \brief Try to resolve strict parameter types
**
** A strict may be given for a func:
**   func foo(p1, p2: StrictTypeP2) // Here, TypeP2 must be resolved
** This is required for func overloading deduction
*/
bool resolveStrictParameterTypes(ny::compiler::Compdb&, Atom& atom, Settings* = nullptr);

} // ny::semantic
