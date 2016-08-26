#include "details/ast/tree-index.h"
#include <array>

using namespace Yuni;



namespace Nany
{

	AST::Node* ASTHelper::nodeCreate(enum AST::Rule rule)
	{
		auto* node = new AST::Node{rule};
		node->metadata = Sema::Metadata::create(node);
		return node;
	}


	AST::Node* ASTHelper::nodeAppend(AST::Node& parent, enum AST::Rule rule)
	{
		auto* node = nodeCreate(rule);
		node->offset = parent.offset;
		node->offsetEnd = parent.offsetEnd;
		AST::metadata(node).parent = &parent;
		parent.children.push_back(node);
		return node;
	}


	AST::Node* ASTHelper::nodeAppendAsOriginal(AST::Node& parent, enum AST::Rule rule)
	{
		auto* node = nodeAppend(parent, rule);
		AST::metadata(node).fromASTTransformation = false;
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
		AST::metadata(node).parent = &newParent;
	}


	void ASTHelper::nodeReparentAtTheBegining(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent)
	{
		assert(&node != &newParent and "should not be similar");
		assert(index < oldParent.children.size());

		// add the child first, to keep a reference somewhere
		newParent.children.insert(newParent.children.begin(), &node);
		// metadata, register the new parent
		AST::metadata(node).parent = &newParent;

		// remove it from the old parent
		oldParent.children.erase(oldParent.children.begin() + index);
	}


	void ASTHelper::nodeRulePromote(AST::Node& node, enum AST::Rule rule)
	{
		node.rule = rule;
	}




} // namespace Nany
