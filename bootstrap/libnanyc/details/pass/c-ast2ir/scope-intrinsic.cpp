#include "scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;

namespace ny::ir::Producer {

bool Scope::visitASTExprIntrinsic(AST::Node& node, uint32_t& localvar) {
	assert(node.rule == AST::rgIntrinsic);
	assert(not node.children.empty());
	bool success = true;
	ShortString128 intrinsicname;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgEntity: {
				for (auto& entity : child.children) {
					switch (entity.rule) {
						case AST::rgIdentifier: {
							if (not intrinsicname.empty())
								intrinsicname += '.';
							intrinsicname += entity.text;
							break;
						}
						default: {
							success = unexpectedNode(entity, "[intrinsic/entity]");
							break;
						}
					}
				}
				break;
			}
			case AST::rgIdentifier: {
				intrinsicname = child.text;
				break;
			}
			case AST::rgCall: {
				success &= visitASTExprCallParameters(child);
				break;
			}
			default:
				success = unexpectedNode(child, "[intrinsic]");
		}
	}
	// create a value even if nothing
	emitDebugpos(node);
	auto& irout = ircode();
	localvar = ir::emit::alloc(irout, nextvar());
	ir::emit::intrinsic(irout, localvar, intrinsicname);
	return success;
}

} // ny::ir::Producer
