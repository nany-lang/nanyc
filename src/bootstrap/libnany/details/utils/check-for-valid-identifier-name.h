#pragma once
#include "details/grammar/nany.h"
#include "details/fwd.h"



namespace Nany
{


	/*!
	** \brief Determine if an identifier is acceptable
	*/
	bool checkForValidIdentifierName(Logs::Report& report, const Nany::Node& node, const AnyString& name, bool isOperator);


	/*!
	** \brief Normalize an operator identifier (ex: 'and' to '^and')
	*/
	AnyString normalizeOperatorName(AnyString name);



} // namespace Nany
