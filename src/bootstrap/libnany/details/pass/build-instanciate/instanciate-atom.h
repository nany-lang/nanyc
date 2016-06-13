#pragma once
#include <yuni/yuni.h>
#include "details/context/build.h"
#include "details/reporting/report.h"
#include "details/atom/atom.h"
#include "func-overload-match.h"



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
			: cdeftable(cdeftable)
			, atom(atom)
			, build(build)
			, params(params)
			, tmplparams(tmplparams)
			, report(report)
		{
			returnType.mutateToAny();

			if (!report)
				report = new Logs::Message{Logs::Level::none};
		}

		//! The original view to the classdef table
		ClassdefTableView& cdeftable;

		//! The atom to instanciate
		std::reference_wrapper<Atom> atom;
		//! The parent atom, if any
		Atom* parentAtom = nullptr;
		//! Instance
		uint32_t instanceid = (uint32_t) -1;
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
		Logs::Message::Ptr& report;

	}; // class InstanciateData




	/*!
	** \brief Instanciate atom
	*/
	bool instanciateAtom(InstanciateData& info);





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
