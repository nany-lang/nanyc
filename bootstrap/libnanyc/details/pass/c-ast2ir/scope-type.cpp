#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace ny
{
namespace ir
{
namespace Producer
{


	bool Scope::visitASTExprTypeDecl(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgTypeDecl);
		localvar = 0;

		if (node.children.size() == 1)
		{
			auto& identifier = node.children[0];
			if (identifier.children.empty())
			{
				if (identifier.text == "any") // no real information, already 'any'
				{
					localvar = LVID(-1);
					return true;
				}
				if (identifier.text == "void")
				{
					localvar = 0;
					return true;
				}
			}
			else
			{
				// anonymous / inline class definitnio
				if (identifier.rule == AST::rgClass)
				{
					//error(identifier) << "anonymous classes are not supported yet";
					//return false;
					return visitASTClass(identifier, &localvar);
				}
			}
		}
		ir::Producer::Scope scope{*this};
		OpcodeCodegenDisabler codegenDisabler{sequence()};
		return scope.visitASTExpr(node, localvar);
	}


	bool Scope::visitASTType(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgType);
		assert(not node.children.empty());

		bool success = true;
		bool reallyVoid = false;
		bool isRef = false;
		bool isConst = false;
		localvar = 0; // reset

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgTypeDecl:
				{
					if (unlikely(localvar != 0))
						return unexpectedNode(child, "[ir/new/several calls]");

					bool status = visitASTExprTypeDecl(child, localvar);
					success &= status;

					if (status and localvar == 0)
						reallyVoid = true;
					break;
				}
				case AST::rgTypeQualifier:
				{
					for (auto& qualifier: child.children)
					{
						switch (qualifier.rule)
						{
							case AST::rgRef:   isRef   = true; break;
							case AST::rgConst: isConst = true; break;
							case AST::rgCref:  isRef   = true; isConst = true; break;
							default:
								success = unexpectedNode(child, "[ir/type-qualifier]");
						}
					}
					break;
				}
				case AST::rgClass:
				{
					break;
				}
				default:
					success = unexpectedNode(child, "[ir/type]");
			}
		}

		emitDebugpos(node);
		auto& out = sequence();

		// create a value even if nothing to always have an attached value
		if (localvar == 0 and not reallyVoid /*any*/)
			localvar = out.emitStackalloc(nextvar(), nyt_any);

		if (0 != localvar)
		{
			if (localvar == (uint32_t) -1)
				localvar = out.emitStackalloc(reserveLocalVariable(), nyt_any);

			out.emitQualifierRef(localvar, isRef);
			out.emitQualifierConst(localvar, isConst);
		}
		return success;
	}




} // namespace Producer
} // namespace ir
} // namespace ny
