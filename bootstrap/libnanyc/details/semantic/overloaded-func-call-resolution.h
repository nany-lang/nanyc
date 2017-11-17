#include <yuni/yuni.h>
#include "details/compiler/compdb.h"
#include "details/reporting/report.h"
#include "details/ir/sequence.h"
#include "details/atom/atom.h"
#include "details/atom/signature.h"
#include "details/atom/classdef-table.h"
#include "func-overload-match.h"
#include <memory>
#include <vector>

namespace ny::semantic {

struct Analyzer;

//! Helper for resolving func call
class OverloadedFuncCallResolver final {
public:
	typedef decltype(FuncOverloadMatch::result.params)  ParameterTypesRequested;
	typedef std::pair<ParameterTypesRequested, ParameterTypesRequested> ParameterTypesRequestedPair;

public:
	OverloadedFuncCallResolver(Analyzer* parent, Logs::Report report, FuncOverloadMatch& overloadMatch,
			ClassdefTableView& cdeftable, ny::compiler::Compdb& compdb)
		: overloadMatch(overloadMatch)
		, report(report)
		, cdeftable(cdeftable)
		, compdb(compdb)
		, parent(parent) {
	}

	bool resolve(const std::vector<std::reference_wrapper<Atom>>& solutions);

public:
	//! The result of the func call resolution (if any)
	Atom* atom = nullptr;
	//! Types requested per parameter (to keep traces of implicit object creations)
	ParameterTypesRequested* params = nullptr;
	//! Types requested for template parameters
	ParameterTypesRequested* tmplparams = nullptr;

	bool canGenerateCode = true;
	bool canGenerateErrors = true;

public:
	// total number of suitable solutions (<= suitable.size())
	uint32_t suitableCount = 0;
	//! Flag to determine if a solution is suitable
	std::vector<bool> suitable;

	/*!
	** \brief Scores for each solution
	**
	** The score is merely the number of parameters that match perfectly
	** ex:
	**    func foo(cref a, cref b)            // 1
	**    func foo(cref a, cref b: std.Ascii) // 2
	**    foo(10, 'a');                       // scores: 1: 0, 2: 1 (must call 2)
	** ex:
	**    func foo(cref a: i32, cref b)       // 1
	**    func foo(cref a, cref b: std.Ascii) // 2
	**    foo(10, 'a');                       // scores: 1: 1, 2: 1 (no perfect match)
	*/
	std::vector<uint32_t> scores;

	//! All subreports for suitable solutions
	std::vector<std::unique_ptr<Logs::Message>> subreports;

private:
	FuncOverloadMatch& overloadMatch;
	//! Reporting
	Logs::Report report;
	//! Parameters per solution
	std::vector<ParameterTypesRequestedPair> parameters;
	//! Solutions that can really be instanciated
	std::vector<bool> solutionsThatCanBeInstanciated;
	//!
	ClassdefTableView& cdeftable;
	ny::compiler::Compdb& compdb;
	//! Parent Sequence builder, if any
	Analyzer* parent = nullptr;;

}; // class OverloadedFuncCallResolver

} // ny::semantic
