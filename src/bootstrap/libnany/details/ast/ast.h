#pragma once
#include <yuni/yuni.h>
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
	Node* createNodeFunc(const AnyString& name);

	//! Create a new node 'function' with 1 cref param
	Node* createNodeFuncCrefParam(const AnyString& name, const AnyString& paramname);





} // namespace AST
} // namespace Nany

#include "ast.hxx"
