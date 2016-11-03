#include <yuni/yuni.h>
#include "scope.h"
#include "details/utils/check-for-valid-identifier-name.h"

using namespace Yuni;




namespace ny
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTTypedef(AST::Node& node)
	{
		assert(node.rule == AST::rgTypedef);

		AnyString typedefname;
		AST::Node* typeexpr = nullptr;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgIdentifier:
				{
					typedefname = child.text;
					bool ok = checkForValidIdentifierName(child, typedefname, IdNameFlag::isType);
					if (unlikely(not ok))
						return false;
					break;
				}
				case AST::rgType:
				{
					typeexpr = &child;
					break;
				}
				default:
					return unexpectedNode(child, "[ir/typedef]");
			}
		}

		if (unlikely(nullptr == typeexpr))
			return (ice(node) << "invalid typedef definition");

		auto& out = sequence();

		IR::Producer::Scope scope{*this};
		uint32_t bpoffset = out.emitBlueprintTypealias(typedefname);
		uint32_t bpoffsiz = out.emitBlueprintSize();
		uint32_t bpoffsck = out.emitStackSizeIncrease();

		// making sure that debug info are available
		context.pPreviousDbgLine = (uint32_t) -1; // forcing debug infos
		scope.addDebugCurrentFilename();
		scope.emitDebugpos(node);

		uint32_t returntype = 1u; // already allocated
		uint32_t localvar   = 0;

		bool success = scope.visitASTType(*typeexpr, localvar);
		success &= (localvar > returntype);

		scope.emitDebugpos(node);

		if (success)
		{
			auto& operands    = scope.sequence().emit<ISA::Op::follow>();
			operands.follower = returntype;
			operands.lvid     = localvar;
			operands.symlink  = 0;
		}

		out.emitPragmaFuncBody(); // to mimic other blueprints
		out.emitEnd();
		uint32_t blpsize = out.opcodeCount() - bpoffset;
		out.at<ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		out.at<ISA::Op::stacksize>(bpoffsck).add = scope.nextVarID + 1u;
		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace ny
