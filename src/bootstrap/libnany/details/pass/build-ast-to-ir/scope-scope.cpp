#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprScope(Node& node)
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
		for (auto& childptr: node.children)
			success &= scope.visitASTStmt(*childptr);

		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
