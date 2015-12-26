#pragma once
#include "details/grammar/nany.h"
#include "details/fwd.h"



namespace Nany
{


	bool checkForValidIdentifierName(Logs::Report& report, const Nany::Node& node, const AnyString& name, bool isOperator);


} // namespace Nany
