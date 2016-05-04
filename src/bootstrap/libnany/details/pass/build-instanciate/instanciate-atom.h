#pragma once
#include <yuni/yuni.h>
#include "details/context/build.h"
#include "details/reporting/report.h"
#include "details/atom/atom.h"
#include "details/atom/func-overload-match.h"



namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	class SequenceBuilder;


	struct InstanciateData final
	{
		InstanciateData(Logs::Message::Ptr& report, Atom& atom, ClassdefTableView& cdeftable, Build& build,
			decltype(FuncOverloadMatch::result.params)& params,
			decltype(FuncOverloadMatch::result.params)& tmplparams)
			: report(report)
			, atom(atom)
			, cdeftable(cdeftable)
			, build(build)
			, params(params)
			, tmplparams(tmplparams)
		{
			returnType.mutateToAny();

			if (!report)
				report = new Logs::Message{Logs::Level::none};
		}

		Logs::Message::Ptr& report;
		//! The atom to instanciate
		std::reference_wrapper<Atom> atom;
		//! The parent atom, if any
		Atom* parentAtom = nullptr;
		//! Instance
		uint32_t instanceid = (uint32_t) -1;
		//! The original view to the classdef table
		ClassdefTableView& cdeftable;
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
	};

	bool instanciateAtom(InstanciateData& info);





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
