#pragma once
#include <yuni/yuni.h>
#include "details/fwd.h"
#include "details/atom/classdef.h"
#include "details/atom/classdef-table.h"
#include <functional>
#include <vector>





namespace Nany
{


	class FuncOverloadMatch final
	{
	public:
		struct ParamCall final
		{
			CLID clid;
			//! The lvid to push as parameter (clid/lvid are linked)
			const Classdef* cdef = nullptr;
			//! implicit constructor, if any
			Atom* implicitCtor = nullptr;
		};

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		FuncOverloadMatch(Logs::Report report, const ClassdefTableView& table);
		//! Destructor
		~FuncOverloadMatch() = default;
		//@}

		void clear();

		Match validate(Atom& atom, bool allowImplicit = true);


		void printInputParameters(YString& out) const;


	public:
		struct Input final
		{
			//! The return type
			std::vector<CLID> rettype;
			//! Indexed parameters
			std::vector<CLID> indexedParams;
			//! name parameters
			std::vector<std::pair<AnyString, CLID>> namedParams;
		}
		input;

		struct Result final
		{
			//! Real-atom to call
			Atom* funcToCall = nullptr;
			//! All params
			std::vector<ParamCall> params;
		}
		result;

		//! Flag to enable/disable error reporting
		bool canGenerateReport = true;
		//! Reporting
		mutable std::reference_wrapper<Logs::Report> report;


	private:
		inline bool hasAtLeastOneParameter(Atom& atom) const;
		inline Match pushParameter(Atom& atom, yuint32 index, const CLID& clid);

	private:
		const ClassdefTableView& table;
		bool pAllowImplicit = false;

	}; // class FuncOverloadMatch





} // namespace Nany

#include "func-overload-match.hxx"
