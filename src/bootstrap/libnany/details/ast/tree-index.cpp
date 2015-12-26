#include "details/ast/tree-index.h"
#include <array>

using namespace Yuni;



namespace Nany
{

	//! Index nodes from a particular type ?
	static std::array<bool, (uint) Nany::ruleCount> indexEnabled;



	void ASTHelper::initialize()
	{
		static Mutex mutex;
		static bool isInitialized = false;

		MutexLocker locker(mutex);
		if (not isInitialized)
		{
			isInitialized = true;
			indexEnabled[(uint) rgIf] = true;
			indexEnabled[(uint) rgVar] = true;
			indexEnabled[(uint) rgClass] = true;
			indexEnabled[(uint) rgFunction] = true;
			indexEnabled[(uint) rgTypedef] = true;
			indexEnabled[(uint) rgStringInterpolation] = true;
		}
	}


	void ASTHelper::clear()
	{
		for (uint i = (uint) index.size(); i--; )
			index[i].clear();
	}


	bool ASTHelper::isIndexEnabled(enum Nany::ASTRule rule) const
	{
		return indexEnabled[(uint) rule];
	}


	Nany::Node* ASTHelper::nodeCreate(enum Nany::ASTRule rule)
	{
		auto* node = new Nany::Node{rule};
		if (indexEnabled[(uint) rule])
			index[(uint) rule].insert(node);
		node->metadata = Sema::Metadata::create(node);
		return node;
	}


	Nany::Node* ASTHelper::nodeAppend(Nany::Node& parent, enum Nany::ASTRule rule)
	{
		auto* node = nodeCreate(rule);
		node->offset = parent.offset;
		node->offsetEnd = parent.offsetEnd;
		AST::metadata(node).parent = &parent;
		parent.children.push_back(node);
		return node;
	}


	Nany::Node* ASTHelper::nodeAppendAsOriginal(Nany::Node& parent, enum Nany::ASTRule rule)
	{
		auto* node = nodeAppend(parent, rule);
		AST::metadata(node).fromASTTransformation = false;
		return node;
	}


	inline void ASTHelper::nodeRemoveFromIndex(Nany::Node& node)
	{
		if (indexEnabled[(uint) node.rule])
		{
			auto& set = index[(uint) node.rule];
			auto i = set.find(&node);
			if (i != set.end())
				index[(uint) node.rule].erase(i);
		}
	}


	inline void ASTHelper::nodeAddIndex(Nany::Node& node)
	{
		if (indexEnabled[(uint) node.rule])
			index[(uint) node.rule].insert(&node);
	}


	void ASTHelper::nodeReparentAtTheEnd(Nany::Node& node, Nany::Node& oldParent, uint index, Nany::Node& newParent)
	{
		assert(&node != &newParent and "should not be similar");
		assert(index < oldParent.children.size());

		// acquire node pointer
		Node::Ptr ptr = &node;

		// remove first it from the old parent
		oldParent.children.erase(oldParent.children.begin() + index);

		// add the child first, to keep a reference somewhere
		newParent.children.push_back(ptr);
		// metadata, register the new parent
		AST::metadata(node).parent = &newParent;
	}


	void ASTHelper::nodeReparentAtTheBegining(Nany::Node& node, Nany::Node& oldParent, uint index, Nany::Node& newParent)
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


	void ASTHelper::nodeRulePromote(Nany::Node& node, enum Nany::ASTRule rule)
	{
		node.rule = rule;
	}




} // namespace Nany
