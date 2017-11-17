#include <yuni/yuni.h>
#include "scope.h"
#include "details/utils/check-for-valid-identifier-name.h"

using namespace Yuni;

namespace ny::ir::Producer {

bool Scope::visitASTTypedef(AST::Node& node) {
	assert(node.rule == AST::rgTypedef);
	AnyString typedefname;
	AST::Node* typeexpr = nullptr;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgIdentifier: {
				typedefname = child.text;
				bool ok = checkForValidIdentifierName(child, typedefname, IdNameFlag::isType);
				if (unlikely(not ok))
					return false;
				break;
			}
			case AST::rgType: {
				typeexpr = &child;
				break;
			}
			default:
				return unexpectedNode(child, "[ir/typedef]");
		}
	}
	if (unlikely(nullptr == typeexpr))
		return (ice(node) << "invalid typedef definition");
	auto& irout = ircode();
	ir::Producer::Scope scope{*this};
	uint32_t bpoffset = ir::emit::blueprint::typealias(irout, typedefname);
	uint32_t bpoffsiz = ir::emit::pragma::blueprintSize(irout);
	uint32_t bpoffsck = ir::emit::increaseStacksize(irout);
	// making sure that debug info are available
	context.invalidateLastDebugLine(); // forcing debug infos
	ir::emit::dbginfo::filename(irout, context.dbgSourceFilename);
	scope.emitDebugpos(node);
	uint32_t returntype = 1u; // already allocated
	uint32_t localvar   = 0;
	bool success = scope.visitASTType(*typeexpr, localvar);
	success &= (localvar > returntype);
	scope.emitDebugpos(node);
	if (success) {
		auto& operands    = irout.emit<isa::Op::follow>();
		operands.follower = returntype;
		operands.lvid     = localvar;
		operands.symlink  = 0;
	}
	ir::emit::pragma::funcbody(irout); // to mimic other blueprints
	ir::emit::scopeEnd(irout);
	uint32_t blpsize = irout.opcodeCount() - bpoffset;
	irout.at<isa::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
	irout.at<isa::Op::stacksize>(bpoffsck).add = scope.nextVarID + 1u;
	return success;
}

} // ny::ir::Producer
