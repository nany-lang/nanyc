#include <yuni/yuni.h>
#include "scope.h"
#include "details/ast/ast.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"

using namespace Yuni;




namespace ny
{
namespace ir
{
namespace Producer
{


	bool Scope::visitASTExprNew(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgNew);
		if (unlikely(node.children.empty()))
			return false;

		bool success = true;

		// create a new variable for the result
		LVID rettype = 0;
		AST::Node* call = nullptr;
		AST::Node* inplace = nullptr;

		for (auto& child: node.children)
		{
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
				case AST::rgNewShared:
				{
					error(child) << "'new shared': Distributed objects are not implemented yet";
					success = false;
					break;
				}
				case AST::rgNewParameters:
				{
					for (auto& param: child.children)
					{
						if (unlikely(param.rule != AST::rgNewNamedParameter or param.children.size() != 2))
							return unexpectedNode(param, "[ir/new/params]");

						auto& pname = param.children[0];
						auto& pexpr = param.children[1];
						if (unlikely(pname.rule != AST::rgIdentifier or pexpr.rule != AST::rgExpr))
							return unexpectedNode(pname, "[ir/new/param/id]");

						if (pname.text == "inplace")
						{
							inplace = &pexpr;
						}
						else
						{
							error(pname) << "unknown 'new' parameter '" << pname.text << "'";
							success = false;
						}
					}
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
		auto& out = sequence();
		ir::emit::type::isobject(out, rettype);

		// OBJECT ALLOCATION
		uint32_t pointer = 0;
		if (inplace == nullptr)
		{
			// SIZEOF
			// trick: a register will be allocated even if unused for now
			// it will be later (when instanciating the func) to put the sizeof
			// of the object to allocate
			ir::emit::alloc(out, nextvar(), nyt_u64);
			pointer = ir::emit::alloc(out, nextvar());
			ir::emit::objectAlloc(out, pointer, rettype);
		}
		else
		{
			uint32_t inplaceExpr = 0;
			if (not visitASTExpr(*inplace, inplaceExpr))
				return false;
			// intermediate pointer to force type __pointer
			uint32_t tmpptr = ir::emit::alloc(out, nextvar(), nyt_ptr);
			out.emitAssign(tmpptr, inplaceExpr, false);
			// promoting the given __pointer to T
			pointer = ir::emit::alloc(out, nextvar());
			ir::emit::copy(out, pointer, tmpptr);
		}

		// type propagation
		auto& operands    = out.emit<ISA::Op::follow>();
		operands.follower = pointer;
		operands.lvid     = rettype;
		operands.symlink  = 0;

		localvar = pointer;

		// CONSTRUCTOR CALL
		uint32_t lvidcall = ir::emit::alloc(out, nextvar());
		ir::emit::identify(out, lvidcall, "^new", pointer);
		return visitASTExprCall(call, lvidcall);
	}




} // namespace Producer
} // namespace ir
} // namespace ny
