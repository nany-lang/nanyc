#include "details/ast/tree-index.h"
#include <array>

using namespace Yuni;



namespace Nany
{

	AST::Node* ASTHelper::nodeAppend(AST::Node& parent, enum AST::Rule rule)
	{
		auto* node = nodeCreate(rule);
		node->offset = parent.offset;
		node->offsetEnd = parent.offsetEnd;
		node->parent = &parent;
		parent.children.push_back(node);
		return node;
	}


	void ASTHelper::nodeReparentAtTheEnd(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent)
	{
		assert(&node != &newParent and "should not be similar");
		assert(index < oldParent.children.size());

		// acquire node pointer
		AST::Node::Ptr ptr = &node;

		// remove first it from the old parent
		oldParent.children.erase(oldParent.children.begin() + index);

		// add the child first, to keep a reference somewhere
		newParent.children.push_back(ptr);
		// metadata, register the new parent
		node.parent = &newParent;
	}


	void ASTHelper::nodeReparentAtTheBegining(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent)
	{
		assert(&node != &newParent and "should not be similar");
		assert(index < oldParent.children.size());

		// add the child first, to keep a reference somewhere
		newParent.children.insert(newParent.children.begin(), &node);
		// metadata, register the new parent
		node.parent = &newParent;

		// remove it from the old parent
		oldParent.children.erase(oldParent.children.begin() + index);
	}




} // namespace Nany
