#pragma once
#include <yuni/yuni.h>
#include "details/reporting/report.h"
#include "details/atom/func-overload-match.h"
#include "details/ir/sequence.h"
#include "details/atom/atom.h"
#include "details/atom/signature.h"
#include "details/atom/classdef-table.h"
#include "nany/nany.h"
#include <memory>
#include <vector>



namespace Nany
{

	/*!
	** \brief Helper for resolving func call
	*/
	class OverloadedFuncCallResolver final
	{
	public:
		typedef decltype(FuncOverloadMatch::result.params)  ParameterTypesRequested;
		typedef std::pair<ParameterTypesRequested, ParameterTypesRequested> ParameterTypesRequestedPair;


	public:
		OverloadedFuncCallResolver(Logs::Report report, FuncOverloadMatch& overloadMatch, ClassdefTableView& cdeftable,
			nycontext_t& context)
			: overloadMatch(overloadMatch)
			, report(report)
			, cdeftable(cdeftable)
			, context(context)
		{}

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
		//! All suitable solutions
		std::vector<bool> suitable;
		// total number of suitable solutions (<= suitable.size())
		uint suitableCount = 0;

		//! All subreports for suitable solutions
		std::vector<Logs::Message::Ptr> subreports;


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
		//! Parent context
		nycontext_t& context;

	}; // class OverloadedFuncCallResolver






} // namespace Nany
