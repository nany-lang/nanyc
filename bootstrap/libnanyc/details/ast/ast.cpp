#include "details/ast/ast.h"

using namespace Yuni;


namespace ny {
namespace AST {


Ref<Node> createNodeFunc(Ref<Node>& funcname) {
	auto func = make_ref<Node>(rgFunction);
	// no visibility, using the default one
	// // visibility: public
	// // funcGetNode.children.push_back(new Node{rgVisibility, "public"});
	// function-kind
	// |   function-kind-function (+2)
	// |       symbol-name
	// |           identifier: main
	auto funcKind = make_ref<Node>(rgFunctionKind);
	func->children.push_back(funcKind);
	{
		auto funcKindFunc = make_ref<Node>(rgFunctionKindFunction);
		funcKind->children.push_back(funcKindFunc);
		{
			auto symname = make_ref<Node>(rgSymbolName);
			funcKindFunc->children.push_back(symname);
			funcname = createNodeIdentifier(nullptr);
			symname->children.push_back(funcname);
		}
	}
	return func;
}


Ref<Node> createNodeFuncCrefParam(Ref<Node>& funcname, const AnyString& paramname) {
	auto func = createNodeFunc(funcname);
	// func-params (+3)
	// |   func-param (+2)
	// |   |   cref
	// |   |   identifier: rhs
	auto params = make_ref<Node>(rgFuncParams);
	func->children.push_back(params);
	auto firstParam = make_ref<Node>(rgFuncParam);
	params->children.push_back(firstParam);
	firstParam->children.push_back(new Node{rgCref});
	firstParam->children.push_back(createNodeIdentifier(paramname));
	return func;
}

void nodeReparentAtTheEnd(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent) {
	assert(&node != &newParent and "should not be similar");
	assert(index < oldParent.children.size());
	// acquire node pointer
	Ref<AST::Node> ptr = &node;
	// remove first it from the old parent
	oldParent.children.erase(index);
	// add the child first, to keep a reference somewhere
	newParent.children.push_back(ptr);
	// metadata, register the new parent
	node.parent = &newParent;
}

void nodeReparentAtTheBegining(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent) {
	assert(&node != &newParent and "should not be similar");
	assert(index < oldParent.children.size());
	// add the child first, to keep a reference somewhere
	newParent.children.push_front(&node);
	// metadata, register the new parent
	node.parent = &newParent;
	// remove it from the old parent
	oldParent.children.erase(index);
}


} // namespace AST
} // namespace ny
