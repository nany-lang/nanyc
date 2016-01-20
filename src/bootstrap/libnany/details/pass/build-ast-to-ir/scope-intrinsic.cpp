#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprIntrinsic(Node& node, LVID& localvar)
	{
		assert(node.rule == rgIntrinsic);
		assert(not node.children.empty());

		bool success = true;
		ShortString128 intrinsicname;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case rgEntity:
				{
					for (auto& entityptr: child.children)
					{
						auto& entity = *entityptr;
						switch (entity.rule)
						{
							case rgIdentifier:
							{
								if (not intrinsicname.empty())
									intrinsicname += '.';
								intrinsicname += entity.text;
								break;
							}
							default:
							{
								success = ICEUnexpectedNode(entity, "[intrinsic/entity]");
								break;
							}
						}
					}
					break;
				}

				case rgIdentifier:
				{
					intrinsicname = child.text;
					break;
				}

				case rgCall:
				{
					success &= visitASTExprCallParameters(child);
					break;
				}

				default:
					success = ICEUnexpectedNode(child, "[intrinsic]");
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
