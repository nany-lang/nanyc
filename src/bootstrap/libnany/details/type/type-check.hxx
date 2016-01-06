#pragma once
#include "type-check.h"


namespace Nany
{
namespace TypeCheck
{


	inline Match isSimilarTo(const ClassdefTableView& table, const CTarget* target,
		const Classdef& A, const Classdef& B, bool allowImplicit)
	{
		return isSimilarTo(table.originalTable(), target, A, B, allowImplicit);
	}




} // namespace TypeCheck
} // namespace Nany
