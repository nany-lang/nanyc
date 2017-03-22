#include "scope.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


namespace {


bool extractParameterDetails(Scope& scope, AST::Node& node, uint32_t& lvid, AnyString& name) {
	bool isRef = false;
	bool isConst = false;
	AST::Node* typeNode = nullptr;
	for (auto& child: node.children) {
		switch (child.rule) {
			case AST::rgIdentifier: {
				name = child.text;
				break;
			}
			case AST::rgRef:
				isRef   = true;
				break;
			case AST::rgConst:
				isConst = true;
				break;
			case AST::rgCref:
				isRef   = true;
				isConst = true;
				break;
			case AST::rgVarType: {
				for (auto& typeChild: child.children) {
					switch (typeChild.rule) {
						case AST::rgType: {
							typeNode = &typeChild;
							break;
						}
						default:
							return unexpectedNode(child, "on/scope/fail/param/details/type");
					}
				}
				break;
			}
			default:
				return unexpectedNode(child, "on/scope/fail/param/details");
		}
	}
	if (typeNode != nullptr) {
		if (!scope.visitASTType(*typeNode, lvid))
			return false;
		switch (lvid) {
			default: {
				if (isRef)
					ir::emit::type::qualifierRef(scope.ircode(), lvid, true);
				if (isConst)
					ir::emit::type::qualifierConst(scope.ircode(), lvid, true);
				break;
			}
			case (uint32_t) -1: {
				lvid = 0; // let's pretend no type was given
				break;
			}
			case 0: // void
				return error(*typeNode) << "void is not a valid parameter type";
		}
	}
	return true;
}


bool findParameter(Scope& scope, AST::Node& node, uint32_t& lvid, AnyString& name) {
	bool hasParameter = false;
	for (auto& child: node.children) {
		if (unlikely(child.rule != AST::rgFuncParams))
			return unexpectedNode(child, "on/scope/fail");
		for (auto& paramsChild: child.children) {
			if (unlikely(paramsChild.rule != AST::rgFuncParam))
				return unexpectedNode(paramsChild, "on/scope/fail/params");
			if (unlikely(hasParameter))
				return error(paramsChild) << "'on scope fail' accepts only one parameter";
			hasParameter = true;
			if (!extractParameterDetails(scope, paramsChild, lvid, name))
				return false;
		}
	}
	return true;
}


} // namespace


bool Scope::visitASTExprOnScopeFail(AST::Node& scopeNode, AST::Node& scopeFailNode) {
	assert(scopeNode.rule == AST::rgScope);
	assert(scopeFailNode.rule == AST::rgOnScopeFail);
	uint32_t lvidType = 0;
	AnyString name;
	if (!findParameter(*this, scopeFailNode, lvidType, name))
		return false;
	return true;
}


} // namespace Producer
} // namespace ir
} // namespace ny
