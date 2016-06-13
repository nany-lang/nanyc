#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/ast/ast.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprNew(const AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgNew);
		if (unlikely(node.children.empty()))
			return false;

		bool success = true;

		// create a new variable for the result
		LVID rettype = 0;
		const AST::Node* call = nullptr;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case AST::rgTypeDecl:
				{
					if (unlikely(rettype != 0))
						return unexpectedNode(child, "[ir/new/several calls]");

					success &= visitASTExprTypeDecl(child, rettype);
					if (success)
					{
						if (unlikely(lvidIsAny(rettype)))
						{
							error(child) << "the pseudo type 'any' can not be instanciated";
							return false;
						}
						if (unlikely(rettype == 0))
						{
							error(child) << "the pseudo type 'void' can not be instanciated";
							return false;
						}
					}
					break;
				}
				case AST::rgCall:
				{
					call = &child;
					break;
				}

				default:
					return unexpectedNode(child, "[ir/new]");
			}
		}

		if (unlikely(not success))
			return false;


		// debug info
		emitDebugpos((call ? *call : node));

		sequence().emitTypeIsObject(rettype);

		// trick: a register will be allocated even if unused for now
		// it will be later (when instanciating the func) to put the sizeof
		// of the object to allocate
		sequence().emitStackalloc(nextvar(), nyt_u64);
		// the object allocation itself
		uint32_t pointer = sequence().emitStackalloc(nextvar(), nyt_any);
		sequence().emitAllocate(pointer, rettype);
		localvar = pointer;

		auto& operands    = sequence().emit<ISA::Op::follow>();
		operands.follower = pointer;
		operands.lvid     = rettype;
		operands.symlink  = 0;

		uint32_t lvidcall = sequence().emitStackalloc(nextvar(), nyt_any);
		sequence().emitIdentify(lvidcall, "^new", pointer);

		return visitASTExprCall(call, lvidcall);
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
