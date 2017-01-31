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
};


} // namespace


bool Scope::fetchAttributes(AST::Node& node) {
	assert(node.rule == AST::rgAttributes);
	try {
		AttributeContext ctx{node};
		for (auto& child : node.children) {
			// checking for node type
			if (unlikely(child.rule != AST::rgAttributesParameter))
				throw UnexpectedNode(child, "invalid node, not attribute parameter");
			AST::Node* nodevalue;
			switch (child.children.size()) {
				case 1:
					nodevalue = nullptr;
					break;
				case 2:
					nodevalue = &(child.children[1]);
					break;
				default:
					throw UnexpectedNode(child, "invalid attribute parameter node");
			}
			AST::Node& nodekey = child.children[0];
			if (unlikely(nodekey.rule != AST::rgEntity))
				throw UnexpectedNode(child, "invalid attribute parameter name type");
			if (nodevalue) {
				if (unlikely(nodevalue->rule != AST::rgEntity))
					return (error(child) << "unsupported expression for attribute ctx.value");
			}
			ctx.name.clear();
			ctx.value.clear();
			if (not AST::appendEntityAsString(ctx.name, nodekey))
				throw UnexpectedNode(child, "invalid entity");
			switch (ctx.name[0]) {
				case 'n': {
					if (ctx.name == "nodiscard") {
						if (unlikely(nodevalue))
							return (error(child) << "no ctx.value expected for attribute '" << ctx.name << '\'');
						break;
					}
				}
				// [[fallthru]]
				case 'p': {
					if (ctx.name == "per") {
						if (unlikely(!nodevalue))
							return (error(child) << "ctx.value expected for attribute '" << ctx.name << '\'');
						AST::appendEntityAsString(ctx.value, *nodevalue);
						if (ctx.value == "thread")
							warning(child) << "ignored attribute 'per: thread'";
						else if (ctx.value == "process")
							warning(child) << "ignored attribute 'per: process'";
						else
							return (error(child) << "invalid 'per' ctx.value");
						break;
					}
				}
				// [[fallthru]]
				case 's': {
					if (ctx.name == "nosuggest") {
						ctx.attributes.flags += Attributes::Flag::doNotSuggest;
						if (unlikely(nodevalue))
							return (error(child) << "the attribute '" << ctx.name << "' does not accept ctx.values");
						break;
					}
				}
				// [[fallthru]]
				case 't': {
					if (ctx.name == "threadproc") {
						ctx.attributes.flags += Attributes::Flag::threadproc;
						if (unlikely(nodevalue))
							return (error(child) << "the attribute '" << ctx.name << "' does not accept ctx.values");
						break;
					}
				}
				// [[fallthru]]
				case '_': {
					if (ctx.name == "__nanyc_builtinalias") {
						if (unlikely(!nodevalue))
							return (error(child) << "ctx.value expected for attribute '" << ctx.name << '\'');
						ctx.attributes.builtinAlias = nodevalue;
						ctx.attributes.flags += Attributes::Flag::builtinAlias;
						break;
					}
					if (ctx.name == "__nanyc_shortcircuit") {
						if (unlikely(!nodevalue))
							return (error(child) << "ctx.value expected for attribute '" << ctx.name << '\'');
						AST::appendEntityAsString(ctx.value, *nodevalue);
						bool isTrue = (ctx.value == "__true");
						if (not isTrue and (ctx.value.empty() or ctx.value != "__false")) {
							error(child.children[1]) << "invalid shortcircuit ctx.value, expected '__false' or '__true', got '"
													 << ctx.value << "'";
							return false;
						}
						if (isTrue)
							ctx.attributes.flags += Attributes::Flag::shortcircuit;
						break;
					}
					if (ctx.name == "__nanyc_synthetic") {
						ctx.attributes.flags += Attributes::Flag::pushSynthetic;
						if (unlikely(nodevalue))
							return (error(child) << "the attribute '" << ctx.name << "' does not accept ctx.values");
						break;
					}
					// [FALLBACK]
					// ignore vendor specific attributes (starting by '__')
					if (ctx.name.size() > 2 and ctx.name[1] == '_') {
						// emit a warning for the unsupported specific nany attributes
						if (unlikely(ctx.name.startsWith("__nanyc_")))
							warning(child) << "unknown nanyc attribute '" << ctx.name << "'";
						break;
					}
				}
				// [[fallthru]]
				default: {
					error(child) << "unknown attribute '" << ctx.name << '\'';
					return false;
				}
			}
		}
		attributes = std::move(ctx.storage);
		return true;
	}
	catch (const UnexpectedNode& e) {
		unexpectedNode(e.node, e.message);
	}
	return false;
}


} // namespace Producer
} // namespace ir
} // namespace ny
