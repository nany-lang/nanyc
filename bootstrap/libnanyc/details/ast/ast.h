#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/grammar/nany.h"

namespace ny {
namespace AST {

//! Create a new node 'identifier' ()
yuni::Ref<Node> createNodeIdentifier(const AnyString& name);

//! Create a new node 'function'
yuni::Ref<Node> createNodeFunc(yuni::Ref<Node>& funcname);

//! Create a new node 'function' with 1 cref param
yuni::Ref<Node> createNodeFuncCrefParam(yuni::Ref<Node>& funcname, const AnyString& paramname);

//! Extract the complete identifier string (from an entity node)
template<class S> bool appendEntityAsString(S& out, const Node& node);

//! Copy the offset attributes
void nodeCopyOffsetText(AST::Node& dest, const AST::Node& source);

//! Copy the offset and originalNode attributes
void nodeCopyOffsetAndOriginalNode(AST::Node& dest, const AST::Node& source);

//! Copy the offset + text and originalNode attributes
void nodeCopyOffsetTextAndOriginalNode(AST::Node& dest, const AST::Node& source);

} // namespace AST
} // namespace ny

#include "ast.hxx"
