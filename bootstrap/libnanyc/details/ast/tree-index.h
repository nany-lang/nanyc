#pragma once
#include "libnanyc.h"
#include "ast.h"
#include "details/grammar/nany.h"
#include <array>


namespace ny {


class ASTHelper final {
public:
	//! Create a new node in raw mode (do not set parent/children sets)
	AST::Node* nodeCreate(enum AST::Rule);
	//! Append a new node and append it (+metadata)
	AST::Node* nodeAppend(AST::Node& parent, enum AST::Rule);

	//! Append a new hierarchy of node and append it (+metadata)
	AST::Node* nodeAppend(AST::Node& parent, std::initializer_list<enum AST::Rule> list);

	//! Create a new node and append it (+metadata)
	AST::Node* nodeAppendAsOriginal(AST::Node& parent, enum AST::Rule);

	void nodeRulePromote(AST::Node& node, enum AST::Rule);

	/*!
	** \param index Child Index of \p node
	*/
	void nodeReparentAtTheEnd(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent);
	void nodeReparentAtTheBegining(AST::Node& node, AST::Node& oldParent, uint index, AST::Node& newParent);


	template<class T>
	static void nodeEachParent(AST::Node& node, const T& callback);

	template<class T>
	static void nodeEachItemInXPath(AST::Node& node, const T& callback);

	//! Copy the offset attributes
	static void nodeCopyOffsetText(AST::Node& dest, const AST::Node& source);

	//! Copy the offset and originalNode attributes
	static void nodeCopyOffsetAndOriginalNode(AST::Node& dest, const AST::Node& source);

	//! Copy the offset + text and originalNode attributes
	static void nodeCopyOffsetTextAndOriginalNode(AST::Node& dest, const AST::Node& source);


}; // class ASTHelper


} // namespace ny

#include "tree-index.hxx"
