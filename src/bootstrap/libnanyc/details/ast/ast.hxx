#pragma once
#include "ast.h"



namespace Nany
{
namespace AST
{


	inline Sema::Metadata& metadata(AST::Node& node)
	{
		assert(node.metadata);
		return *((Sema::Metadata*) node.metadata);
	}


	inline const Sema::Metadata& metadata(const AST::Node& node)
	{
		assert(node.metadata);
		return *((const Sema::Metadata*) node.metadata);
	}


	inline Sema::Metadata& metadata(AST::Node* node)
	{
		assert(node);
		assert(node->metadata);
		return *((Sema::Metadata*) node->metadata);
	}


	inline const Sema::Metadata& metadata(const AST::Node* node)
	{
		assert(node);
		assert(node->metadata);
		return *((const Sema::Metadata*) node->metadata);
	}


	inline Sema::Metadata& metadata(AST::Node::Ptr& node)
	{
		assert(!(!node));
		assert(node->metadata);
		return *((Sema::Metadata*) node->metadata);
	}


	inline Node* createNodeIdentifier(const AnyString& name, bool withMetadata)
	{
		if (not withMetadata)
		{
			return new Node{rgIdentifier, name};
		}
		else
		{
			auto* identifier = new Node{rgIdentifier, name};
			identifier->metadata = Sema::Metadata::create(Node::Ptr::WeakPointer(identifier));
			metadata(identifier).fromASTTransformation = true; // to avoid name checking
			return identifier;
		}
	}


	template<class S> bool retrieveEntityString(S& out, const Node& node)
	{
		assert(node.rule == rgEntity);
		out.clear();
		for (auto& ptr: node.children)
		{
			auto& child = *ptr;
			if (YUNI_UNLIKELY(child.rule != rgIdentifier))
				return false;
			if (not out.empty())
				out += '.';
			out += child.text;
		}
		return true;
	}



} // namespace AST
} // namespace Nany
