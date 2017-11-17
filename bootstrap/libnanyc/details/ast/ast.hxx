#pragma once
#include "ast.h"

namespace ny::AST {

inline yuni::Ref<Node> createNodeIdentifier(const AnyString& name) {
	return yuni::make_ref<Node>(rgIdentifier, name);
}

template<class S> bool appendEntityAsString(S& out, const Node& node) {
	assert(node.rule == rgEntity);
	uint32_t count = node.children.size();
	assert(count > 0);
	out += node.children.front().text;
	if (count != 1) {
		for (uint32_t i = 1; i != count; ++i) {
			auto& child = node.children[i];
			if (YUNI_UNLIKELY(child.rule != rgIdentifier))
				return false;
			out += '.';
			out += child.text;
		}
	}
	return true;
}

inline void nodeCopyOffsetText(AST::Node& dest, const AST::Node& source) {
	dest.offset    = source.offset;
	dest.offsetEnd = source.offsetEnd;
	dest.text      = source.text;
}

inline void nodeCopyOffsetAndOriginalNode(AST::Node& dest, const AST::Node& source) {
	dest.offset    = source.offset;
	dest.offsetEnd = source.offsetEnd;
}

inline void nodeCopyOffsetTextAndOriginalNode(AST::Node& dest, const AST::Node& source) {
	dest.offset    = source.offset;
	dest.offsetEnd = source.offsetEnd;
	dest.text      = source.text;
}

} // ny::AST
