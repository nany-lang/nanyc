#pragma once
#include "ast.h"


namespace ny {
namespace AST {


inline Node* createNodeIdentifier(const AnyString& name) {
	return new Node{rgIdentifier, name};
}


template<class S> bool appendEntityAsString(S& out, const Node& node) {
	assert(node.rule == rgEntity);
	assert(node.children.size() > 0);
	out += node.children.front().text;
	if (node.children.size() != 1) {
		for (uint32_t i = 1; i != node.children.size(); ++i) {
			auto& child = node.children[i];
			if (YUNI_UNLIKELY(child.rule != rgIdentifier))
				return false;
			out += '.';
			out += child.text;
		}
	}
	return true;
}


} // namespace AST
} // namespace ny
