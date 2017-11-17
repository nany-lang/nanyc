#include "scope.h"
#include <yuni/core/noncopyable.h>
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include "exception.h"

using namespace Yuni;

namespace ny {
namespace ir {
namespace Producer {

namespace {

struct AttributeContext final {
	AttributeContext(AST::Node& node)
		: storage{std::make_unique<Attributes>(node)}
		, attributes(*storage)
	{}
	ShortString32 name;
	ShortString32 value;
	std::unique_ptr<Attributes> storage;
	Attributes& attributes;
	AST::Node* nodevalue = nullptr;
};

inline AST::Node* getNodeValue(AST::Node& node) {
	switch (node.children.size()) {
		case 1: return nullptr;
		case 2: return &(node.children[1]);
	}
	throw UnexpectedNode(node, "invalid attribute parameter node");
}

void nodiscard(AST::Node&, AttributeContext&) {
	// not implemented
}

void per(AST::Node& node, AttributeContext& ctx) {
	AST::appendEntityAsString(ctx.value, *ctx.nodevalue);
	if (ctx.value == "thread")
		warning(node) << "ignored attribute 'per: thread'";
	else if (ctx.value == "process")
		warning(node) << "ignored attribute 'per: process'";
	else
		throw AttributeError{node, "invalid attribute value (expecting 'thread' or 'process')"};
}

void nosuggest(AST::Node&, AttributeContext& ctx) {
	ctx.attributes.flags += Attributes::Flag::doNotSuggest;
}

void threadproc(AST::Node&, AttributeContext& ctx) {
	ctx.attributes.flags += Attributes::Flag::threadproc;
}

void builtinalias(AST::Node&, AttributeContext& ctx) {
	ctx.attributes.builtinAlias = ctx.nodevalue;
	ctx.attributes.flags += Attributes::Flag::builtinAlias;
}

void shortcircuit(AST::Node& node, AttributeContext& ctx) {
	AST::appendEntityAsString(ctx.value, *ctx.nodevalue);
	bool isTrue = (ctx.value == "__true");
	if (not isTrue and (ctx.value.empty() or ctx.value != "__false"))
		throw UnexpectedNode{node.children[1], "invalid attribute value (expecting '__false' or '__true')"};
	if (isTrue)
		ctx.attributes.flags += Attributes::Flag::shortcircuit;
}

void synthetic(AST::Node&, AttributeContext& ctx) {
	ctx.attributes.flags += Attributes::Flag::pushSynthetic;
}

} // namespace

bool Scope::fetchAttributes(AST::Node& node) {
	assert(node.rule == AST::rgAttributes);
	using Callback = void (*)(AST::Node&, AttributeContext&);
	static const std::unordered_map<AnyString, std::pair<bool, Callback>> dispatch = {
		{"nodiscard",            { false, nodiscard}},
		{"per",                  { true,  per}},
		{"nosuggest",            { false, nosuggest}},
		{"threadproc",           { false, threadproc}},
		//
		{"__nanyc_builtinalias", { true,  builtinalias}},
		{"__nanyc_shortcircuit", { true,  shortcircuit}},
		{"__nanyc_synthetic",    { false, synthetic}}
	};
	try {
		AttributeContext ctx{node};
		for (auto& child : node.children) {
			// checking for node type
			if (unlikely(child.rule != AST::rgAttributesParameter))
				throw UnexpectedNode(child, "invalid node, not attribute parameter");
			ctx.nodevalue = getNodeValue(child);
			AST::Node& nodekey = child.children[0];
			if (unlikely(nodekey.rule != AST::rgEntity))
				throw UnexpectedNode(child, "invalid attribute parameter name type");
			ctx.name.clear();
			ctx.value.clear();
			bool hasName = AST::appendEntityAsString(ctx.name, nodekey);
			if (unlikely(not hasName))
				throw UnexpectedNode(child, "invalid entity");
			auto it = dispatch.find(ctx.name);
			if (unlikely(it == dispatch.end()))
				throw AttributeError{child, "unknown attribute"};
			if (not it->second.first) {
				if (unlikely(ctx.nodevalue))
					throw AttributeError{child, "the attribute does not accept any value"};
			}
			else {
				if (unlikely(ctx.nodevalue == nullptr))
					throw AttributeError{child, "the attribute requires a value"};
			}
			if (ctx.nodevalue) {
				if (unlikely(ctx.nodevalue->rule != AST::rgEntity))
					throw UnexpectedNode{child, "unsupported node type attribute value"};
			}
			it->second.second(child, ctx);
		}
		attributes = std::move(ctx.storage);
		return true;
	}
	catch (const UnexpectedNode& e) {
		unexpectedNode(e.node, e.message);
	}
	catch (const AttributeError& e) {
		error(e.node) << '\'' << e.attrname << "': " << e.message;
	}
	catch (const Error& e) {
		error(e.node) << e.message;
	}
	return false;
}

} // namespace Producer
} // namespace ir
} // namespace ny
