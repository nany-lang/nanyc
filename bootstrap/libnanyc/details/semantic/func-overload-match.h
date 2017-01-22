#pragma once
#include "libnanyc.h"
#include "details/atom/classdef.h"
#include "details/atom/classdef-table.h"
#include "type-check.h"
#include <functional>
#include <vector>


namespace ny {
namespace semantic {


struct SequenceBuilder;


class FuncOverloadMatch final {
public:
	struct ParamCall final {
		CLID clid;
		//! The lvid to push as parameter (clid/lvid are linked)
		const Classdef* cdef = nullptr;

		TypeCheck::Match strategy = TypeCheck::Match::none;
		//! implicit constructor, if any
		Atom* implicitCtor = nullptr;
	};

public:
	//! \name Constructor & Destructor
	//@{
	//! Default constructor
	FuncOverloadMatch(SequenceBuilder*);
	// No copy constructor
	FuncOverloadMatch(const FuncOverloadMatch&) = delete;
	//@}

	/*!
	** \brief Clear all input/output parameters
	*/
	void clear();

	/*!
	** \brief Try to determine if the given func can be called given the input param types
	*/
	TypeCheck::Match validate(Atom& atom, bool allowImplicit = true);

	/*!
	** \brief Try to determine if the given func can be called given the input param types
	** and generate an error report if any error is encountered
	*/
	TypeCheck::Match validateWithErrReport(Atom& atom, bool allowImplicit = true);

	/*!
	** \brief Export input parameters
	*/
	void printInputParameters(YString& out) const;


public:
	struct Input final {
		//! The return type
		std::vector<CLID> rettype;

		struct {
			std::vector<CLID> indexed;
			std::vector<std::pair<AnyString, CLID>> named;
		}
		params;

		struct {
			std::vector<CLID> indexed;
			std::vector<std::pair<AnyString, CLID>> named;
		}
		tmplparams;
	}
	input;

	struct Result final {
		//! Real-atom to call
		Atom* funcToCall = nullptr;
		//! All params
		std::vector<ParamCall> params;
		//! All template parameters
		std::vector<ParamCall> tmplparams;

		//! Score if match or partial match
		uint32_t score = 0u;
	}
	result;

	Logs::Report* report = nullptr;


private:
	template<bool withErrorReporting>
	TypeCheck::Match validateAtom(Atom& atom, bool allowImplicit);

	template<bool withErrorReporting, bool IsTmpl>
	inline TypeCheck::Match pushParameter(Atom& atom, uint32_t index, const CLID& clid);
	void complainParamTypeMismatch(bool isGenType, const Classdef&, const Atom&, uint32_t, const Classdef&);

private:
	bool pAllowImplicit = false;
	SequenceBuilder* seq;

}; // class FuncOverloadMatch




} // namespace semantic
} // namespace ny
