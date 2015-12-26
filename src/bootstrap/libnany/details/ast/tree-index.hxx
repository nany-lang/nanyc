#pragma once
#include "tree-index.h"
#include "ast.h"


namespace Nany
{


	template<class T>
	inline bool ASTHelper::each(enum Nany::ASTRule rule, const T& callback)
	{
		assert(isIndexEnabled(rule) and "index not enabled");
		auto& set = index[(uint) rule];
		auto end = set.end();
		for (auto i = set.begin(); i != end; ++i)
		{
			if (not callback(*(*i)))
				return false;
		}
		return true;
	}


	inline void ASTHelper::nodeCopyOffsetText(Nany::Node& dest, const Nany::Node& source)
	{
		dest.offset    = source.offset;
		dest.offsetEnd = source.offsetEnd;
		dest.text      = source.text;
	}


	inline void ASTHelper::nodeCopyOffsetAndOriginalNode(Nany::Node& dest, const Nany::Node& source)
	{
		dest.offset    = source.offset;
		dest.offsetEnd = source.offsetEnd;
		AST::metadata(dest).originalNode = &source;
	}


	inline void ASTHelper::nodeCopyOffsetTextAndOriginalNode(Nany::Node& dest, const Nany::Node& source)
	{
		dest.offset    = source.offset;
		dest.offsetEnd = source.offsetEnd;
		dest.text      = source.text;
		AST::metadata(dest).originalNode = &source;
	}


	template<class T>
	void ASTHelper::nodeEachParent(Nany::Node& node, const T& callback)
	{
		auto* parent = AST::metadata(node).parent;
		while (parent)
		{
			if (not callback(*parent))
				break;
			parent = AST::metadata(parent).parent;
		}
	}


	template<class T>
	void ASTHelper::nodeEachItemInXPath(Nany::Node& node, const T& callback)
	{
		enum
		{
			revStackHardCodedSize = 64,
		};
		Nany::Node* reverseStack[revStackHardCodedSize];
		uint index = 0;
		// when the stack size is greater than `revStackHardCodedSize`
		std::vector<Nany::Node*> reverseDynStack;

		auto* parent = AST::metadata(node).parent;
		while (parent)
		{
			reverseStack[index] = parent;
			if (YUNI_UNLIKELY(++index >= revStackHardCodedSize))
			{
				// switching to dynamic stack mode
				reverseDynStack.reserve(32);

				parent = AST::metadata(parent).parent;
				while (parent)
				{
					reverseDynStack.push_back(parent);
					parent = AST::metadata(parent).parent;
				}

				for (auto it = reverseDynStack.rbegin(); it != reverseDynStack.rend(); ++it)
				{
					if (not callback(*(*it)))
						return;
				}
				break;
			}

			parent = AST::metadata(parent).parent;
		}

		if (index)
		{
			do
			{
				--index;
				if (not callback(*reverseStack[index]))
					break;
				if (0 == index)
					break;
			}
			while (true);
		}
	}


	inline Nany::Node* ASTHelper::nodeAppend(Nany::Node& parent, std::initializer_list<enum Nany::ASTRule> list)
	{
		Nany::Node* node = &parent;
		for (auto it: list)
			node = nodeAppend(*node, it);
		return node;
	}





} // namespace Nany
