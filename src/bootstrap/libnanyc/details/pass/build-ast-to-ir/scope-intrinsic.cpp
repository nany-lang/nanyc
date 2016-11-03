#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprIntrinsic(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgIntrinsic);
		assert(not node.children.empty());
		bool success = true;
		ShortString128 intrinsicname;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgEntity:
				{
					for (auto& entity: child.children)
					{
						switch (entity.rule)
						{
							case AST::rgIdentifier:
							{
								if (not intrinsicname.empty())
									intrinsicname += '.';
								intrinsicname += entity.text;
								break;
							}
							default:
							{
								success = unexpectedNode(entity, "[intrinsic/entity]");
								break;
							}
						}
					}
					break;
				}
				case AST::rgIdentifier:
				{
					intrinsicname = child.text;
					break;
				}
				case AST::rgCall:
				{
					success &= visitASTExprCallParameters(child);
					break;
				}
				default:
					success = unexpectedNode(child, "[intrinsic]");
			}
		}
		// create a value even if nothing
		emitDebugpos(node);
		localvar = sequence().emitStackalloc(nextvar(), nyt_any);
		sequence().emitIntrinsic(localvar, intrinsicname);
		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
