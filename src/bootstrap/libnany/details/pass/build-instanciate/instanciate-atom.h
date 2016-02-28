#pragma once
#include <yuni/yuni.h>
#include "details/reporting/report.h"



namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	struct InstanciateData final
	{
		InstanciateData(Logs::Message::Ptr& report, Atom& atom, ClassdefTableView& cdeftable, nycontext_t& context,
			decltype(FuncOverloadMatch::result.params)& params)
			: report(report)
			, atom(atom)
			, cdeftable(cdeftable)
			, context(context)
			, params(params)
		{
			returnType.mutateToAny();
		}

		Logs::Message::Ptr& report;
		//! The atom to instanciate
		Atom& atom;
		//! The parent atom, if any
		Atom* parentAtom = nullptr;
		//! Instance
		uint32_t instanceid = (uint32_t) -1;
		//! The original view to the classdef table
		ClassdefTableView& cdeftable;
		//! Context
		nycontext_t& context;

		//! Parameters used for instanciation
		decltype(FuncOverloadMatch::result.params)& params;
		//!
		Classdef returnType;

		//! Flag to determine whether the code can be generated or not
		bool canGenerateCode = true;
		//! Flag to determine whether the code generate errors or not
		bool canGenerateErrors = true;

		//! Make the layer persistent
		bool shouldMergeLayer = false;
	};

	IR::Sequence* InstanciateAtom(InstanciateData& info);





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
