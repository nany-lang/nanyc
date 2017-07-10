#pragma once
#include "ast.h"


namespace ny {
namespace AST {


inline yuni::Ref<Node> createNodeIdentifier(const AnyString& name) {
	return yuni::make_ref<Node>(rgIdentifier, name);
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

template<class T>
void nodeEachParent(AST::Node& node, const T& callback) {
	auto* parent = node.parent;
	while (parent) {
		if (not callback(*parent))
			break;
		parent = parent->parent;
	}
}

template<class T>
void nodeEachItemInXPath(AST::Node& node, const T& callback) {
	enum {
		revStackHardCodedSize = 64,
	};
	AST::Node* reverseStack[revStackHardCodedSize];
	uint index = 0;
	// when the stack size is greater than `revStackHardCodedSize`
	std::vector<AST::Node*> reverseDynStack;
	auto* parent = node.parent;
	while (parent) {
		reverseStack[index] = parent;
		if (YUNI_UNLIKELY(++index >= revStackHardCodedSize)) {
			// switching to dynamic stack mode
			reverseDynStack.reserve(32);
			parent = parent->parent;
			while (parent) {
				reverseDynStack.push_back(parent);
				parent = parent->parent;
			}
			for (auto it = reverseDynStack.rbegin(); it != reverseDynStack.rend(); ++it) {
				if (not callback(*(*it)))
					return;
			}
			break;
		}
		parent = parent->parent;
	}
	if (index) {
		do {
			--index;
			if (not callback(*reverseStack[index]))
				break;
			if (0 == index)
				break;
		}
		while (true);
	}
}

inline AST::Node* nodeAppend(AST::Node& parent, enum AST::Rule rule) {
	auto* node = new AST::Node(rule);
	node->offset = parent.offset;
	node->offsetEnd = parent.offsetEnd;
	node->parent = &parent;
	parent.children.push_back(node);
	return node;
}

inline AST::Node* nodeAppend(AST::Node& parent, std::initializer_list<enum AST::Rule> list) {
	AST::Node* node = &parent;
	for (auto it : list)
		node = nodeAppend(*node, it);
	return node;
}

inline void nodeRulePromote(AST::Node& node, enum AST::Rule rule) {
	node.rule = rule;
}

inline AST::Node* nodeAppendAsOriginal(AST::Node& parent, enum AST::Rule rule) {
	return nodeAppend(parent, rule);
}

} // namespace AST
} // namespace ny
