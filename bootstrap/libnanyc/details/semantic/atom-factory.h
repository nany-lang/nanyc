#pragma once
#include "libnanyc.h"
#include "details/context/build.h"
#include "details/reporting/report.h"
#include "details/atom/atom.h"
#include "func-overload-match.h"


namespace ny {
namespace semantic {


struct SequenceBuilder;


struct InstanciateData final {
	InstanciateData(std::shared_ptr<Logs::Message>& report, Atom& atom, ClassdefTableView& cdeftable, Build& build,
			decltype(FuncOverloadMatch::result.params)& params,
			decltype(FuncOverloadMatch::result.params)& tmplparams)
		: cdeftable(cdeftable)
		, atom(atom)
		, build(build)
		, params(params)
		, tmplparams(tmplparams)
		, report(report) {
		returnType.mutateToAny();
		if (!report)
			report = std::make_shared<Logs::Message>(Logs::Level::none);
	}

	//! The original view to the classdef table
	ClassdefTableView& cdeftable;

	//! The atom to instanciate
	std::reference_wrapper<Atom> atom;
	//! The parent atom, if any
	Atom* parentAtom = nullptr;
	//! Instance
	uint32_t instanceid = (uint32_t) - 1;
	//! Context
	Build& build;

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
	SequenceBuilder* parent = nullptr;

	//! Error reporting
	std::shared_ptr<Logs::Message>& report;

	bool signatureOnly = false;

}; // class InstanciateData


/*!
** \brief Instanciate atom
*/
bool instanciateAtom(InstanciateData& info);

bool instanciateAtomParameterTypes(InstanciateData& info);


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
bool resolveStrictParameterTypes(Build&, Atom& atom, InstanciateData* = nullptr);


} // namespace semantic
} // namespace ny
