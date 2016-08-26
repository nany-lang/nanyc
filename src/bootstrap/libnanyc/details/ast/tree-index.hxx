#pragma once
#include "tree-index.h"
#include "ast.h"


namespace Nany
{


	inline void ASTHelper::nodeCopyOffsetText(AST::Node& dest, const AST::Node& source)
	{
		dest.offset    = source.offset;
		dest.offsetEnd = source.offsetEnd;
		dest.text      = source.text;
	}


	inline void ASTHelper::nodeCopyOffsetAndOriginalNode(AST::Node& dest, const AST::Node& source)
	{
		dest.offset    = source.offset;
		dest.offsetEnd = source.offsetEnd;
		AST::metadata(dest).originalNode = &source;
	}


	inline void ASTHelper::nodeCopyOffsetTextAndOriginalNode(AST::Node& dest, const AST::Node& source)
	{
		dest.offset    = source.offset;
		dest.offsetEnd = source.offsetEnd;
		dest.text      = source.text;
		AST::metadata(dest).originalNode = &source;
	}


	template<class T>
	void ASTHelper::nodeEachParent(AST::Node& node, const T& callback)
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
	void ASTHelper::nodeEachItemInXPath(AST::Node& node, const T& callback)
	{
		enum
		{
			revStackHardCodedSize = 64,
		};
		AST::Node* reverseStack[revStackHardCodedSize];
		uint index = 0;
		// when the stack size is greater than `revStackHardCodedSize`
		std::vector<AST::Node*> reverseDynStack;

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


	inline AST::Node*
	ASTHelper::nodeAppend(AST::Node& parent, std::initializer_list<enum AST::Rule> list)
	{
		AST::Node* node = &parent;
		for (auto it: list)
			node = nodeAppend(*node, it);
		return node;
	}




} // namespace Nany
