#pragma once
#include "details/atom/classdef-table-view.h"


namespace Nany
{
namespace TypeCheck
{

	enum class Match
	{
		//! The two types do not match
		none = 0,
		//! The two types are strictly equal (exactly the same type, or any)
		strictEqual,
		//! The two types have the same public interface
		equal,
	};



	/*!
	** \brief Try to tell if 2 types are similar
	**
	** \param target The target where the resolution occurs [optional]
	** \param A The first type
	** \param B The other type
	** \return 'none' if not equal, othe
	** \note B: Only a well-known classdef, with no interface and no follow-ups (and a valid atom)
	*/
	Match isSimilarTo(const ClassdefTable& table, const CTarget* target, const Classdef& A,
		const Classdef& B, bool allowImplicit = false);


	/*!
	** \brief Try to tell if 2 types are similar (table view)
	**
	** \param target The target where the resolution occurs [optional]
	** \param A The first type
	** \param B The other type
	** \return 'none' if not equal, othe
	** \note B: Only a well-known classdef, with no interface and no follow-ups (and a valid atom)
	*/
	Match isSimilarTo(const ClassdefTableView& table, const CTarget* target, const Classdef& A,
		const Classdef& B, bool allowImplicit = false);



} // namespace TypeCheck
} // namespace Nany

#include "type-check.hxx"
