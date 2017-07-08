#include "details/ast/tree-index.h"
#include <array>

using namespace Yuni;


namespace ny {


void ASTHelper::nodeReparentAtTheEnd(AST::Node& node, AST::Node& oldParent, uint index,
									 AST::Node& newParent) {
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


void ASTHelper::nodeReparentAtTheBegining(AST::Node& node, AST::Node& oldParent, uint index,
		AST::Node& newParent) {
	assert(&node != &newParent and "should not be similar");
	assert(index < oldParent.children.size());
	// add the child first, to keep a reference somewhere
	newParent.children.push_front(&node);
	// metadata, register the new parent
	node.parent = &newParent;
	// remove it from the old parent
	oldParent.children.erase(index);
}


} // namespace ny
