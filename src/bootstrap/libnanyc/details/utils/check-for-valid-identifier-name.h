#pragma once
#include "details/grammar/nany.h"
#include "details/fwd.h"
#include <yuni/core/flags.h>



namespace Nany
{

	enum class IdNameFlag
	{
		//! The identifier name represents a standard operator (+, -, *...)
		isOperator,
		//! The identifier name represents a type name
		isType,
	};


	/*!
	** \brief Determine if an identifier is acceptable
	*/
	bool checkForValidIdentifierName(const AST::Node& node, const AnyString& name, Yuni::Flags<IdNameFlag> flags = nullptr);


	/*!
	** \brief Normalize an operator identifier (ex: 'and' to '^and')
	*/
	AnyString normalizeOperatorName(AnyString name);



} // namespace Nany