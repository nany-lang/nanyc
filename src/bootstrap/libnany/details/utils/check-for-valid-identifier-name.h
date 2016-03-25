#pragma once
#include "details/grammar/nany.h"
#include "details/fwd.h"



namespace Nany
{

	/*!
	** \brief Determine if an identifier is acceptable
	*/
	bool checkForValidIdentifierName(Logs::Report& report, const AST::Node& node,
		const AnyString& name, bool isOperator = false, bool isType = false);


	/*!
	** \brief Normalize an operator identifier (ex: 'and' to '^and')
	*/
	AnyString normalizeOperatorName(AnyString name);



} // namespace Nany
