#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/sema/metadata.h"
#include "details/grammar/nany.h"




namespace Nany
{
namespace AST
{

	//! Extract the metadata object
	Sema::Metadata& metadata(Node&);

	//! Extract the metadata object
	const Sema::Metadata& metadata(const Node&);

	//! Extract the metadata object
	Sema::Metadata& metadata(Node*);

	//! Extract the metadata object
	Sema::Metadata& metadata(Node::Ptr&);

	//! Extract the metadata object
	const Sema::Metadata& metadata(const Node*);



	//! Create a new node 'identifier' ()
	Node* createNodeIdentifier(const AnyString& name, bool withMetadata = false);

	//! Create a new node 'function'
	Node* createNodeFunc(Node*& funcname);

	//! Create a new node 'function' with 1 cref param
	Node* createNodeFuncCrefParam(Node*& funcname, const AnyString& paramname);


	//! Extract the complete identifier string (from an entity node)
	template<class S> bool appendEntityAsString(S& out, const Node& node);




} // namespace AST
} // namespace Nany

#include "ast.hxx"
