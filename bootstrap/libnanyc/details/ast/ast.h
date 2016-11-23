#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/grammar/nany.h"


namespace ny {
namespace AST {


//! Create a new node 'identifier' ()
Node* createNodeIdentifier(const AnyString& name);

//! Create a new node 'function'
Node* createNodeFunc(Node*& funcname);

//! Create a new node 'function' with 1 cref param
Node* createNodeFuncCrefParam(Node*& funcname, const AnyString& paramname);


//! Extract the complete identifier string (from an entity node)
template<class S> bool appendEntityAsString(S& out, const Node& node);


} // namespace AST
} // namespace ny

#include "ast.hxx"
