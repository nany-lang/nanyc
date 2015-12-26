#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprStringLiteral(Node& node, LVID& localvar)
	{
		assert(node.rule == rgStringLiteral);
		assert(node.children.empty());

		emitDebugpos(node);
		localvar = program().emitStackallocText(nextvar(), node.text);
		return true;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
