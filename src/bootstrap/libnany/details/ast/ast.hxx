#pragma once
#include "ast.h"



namespace Nany
{
namespace AST
{


	inline Sema::Metadata& metadata(Nany::Node& node)
	{
		assert(node.metadata);
		return *((Sema::Metadata*) node.metadata);
	}


	inline const Sema::Metadata& metadata(const Nany::Node& node)
	{
		assert(node.metadata);
		return *((const Sema::Metadata*) node.metadata);
	}


	inline Sema::Metadata& metadata(Nany::Node* node)
	{
		assert(node);
		assert(node->metadata);
		return *((Sema::Metadata*) node->metadata);
	}


	inline const Sema::Metadata& metadata(const Nany::Node* node)
	{
		assert(node);
		assert(node->metadata);
		return *((const Sema::Metadata*) node->metadata);
	}


	inline Sema::Metadata& metadata(Nany::Node::Ptr& node)
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




} // namespace AST
} // namespace Nany
