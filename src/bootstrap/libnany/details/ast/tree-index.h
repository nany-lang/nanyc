#pragma once
#include <yuni/yuni.h>
#include "ast.h"
#include "details/fwd.h"
#include "details/grammar/nany.h"
#include <array>




namespace Nany
{

	class ASTHelper final
	{
	public:
		//! Initialize internal variables
		static void initialize();


	public:
		//! clear the internal data
		void clear();

		//! Create a new node in raw mode (do not set parent/children sets)
		Nany::Node* nodeCreate(enum Nany::ASTRule);
		//! Append a new node and append it (+metadata)
		Nany::Node* nodeAppend(Nany::Node& parent, enum Nany::ASTRule);

		//! Append a new hierarchy of node and append it (+metadata)
		Nany::Node* nodeAppend(Nany::Node& parent, std::initializer_list<enum Nany::ASTRule> list);

		//! Create a new node and append it (+metadata)
		Nany::Node* nodeAppendAsOriginal(Nany::Node& parent, enum Nany::ASTRule);

		void nodeRulePromote(Nany::Node& node, enum Nany::ASTRule);

		/*!
		** \param index Child Index of \p node
		*/
		void nodeReparentAtTheEnd(Nany::Node& node, Nany::Node& oldParent, uint index, Nany::Node& newParent);
		void nodeReparentAtTheBegining(Nany::Node& node, Nany::Node& oldParent, uint index, Nany::Node& newParent);


		template<class T>
		static void nodeEachParent(Nany::Node& node, const T& callback);

		template<class T>
		static void nodeEachItemInXPath(Nany::Node& node, const T& callback);

		/*!
		** \brief Iterate through all nodes of a given rule
		** \code
		** ast.each(Nany::rgCallParameter, [&](Nany::Node& node) -> bool
		** {
		** });
		** \endcode
		*/
		template<class T> bool each(enum Nany::ASTRule, const T& callback);

		//! Get if the index is enabled for a given rule
		bool isIndexEnabled(enum Nany::ASTRule) const;

		//! Copy the offset attributes
		static void nodeCopyOffsetText(Nany::Node& dest, const Nany::Node& source);

		//! Copy the offset and originalNode attributes
		static void nodeCopyOffsetAndOriginalNode(Nany::Node& dest, const Nany::Node& source);

		//! Copy the offset + text and originalNode attributes
		static void nodeCopyOffsetTextAndOriginalNode(Nany::Node& dest, const Nany::Node& source);



	public:
		//! AST indexes
		std::array<Nany::Node::Set, (uint) Nany::ruleCount> index;


	private:
		void nodeRemoveFromIndex(Nany::Node& node);
		void nodeAddIndex(Nany::Node& node);

	}; // class ASTHelper






} // namespace Nany

#include "tree-index.hxx"
