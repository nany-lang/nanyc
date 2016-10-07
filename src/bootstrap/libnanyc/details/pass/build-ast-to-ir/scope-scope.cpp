#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprScope(AST::Node& node)
	{
		if (unlikely(kind != Kind::kfunc))
		{
			switch (kind)
			{
				case Kind::kclass:
					error(node) << "scopes not allowed in class definition";
					break;
				default:
					error(node) << "unexpected scope declaration";
			}
			return false;
		}

		IR::OpcodeScopeLocker opscope{sequence()};
		IR::Producer::Scope scope{*this};

		bool success = true;
		for (auto& child: node.children)
			success &= scope.visitASTStmt(child);

		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
