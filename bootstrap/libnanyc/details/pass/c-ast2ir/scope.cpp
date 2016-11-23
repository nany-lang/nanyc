#include "scope.h"
#include "details/grammar/nany.h"
#include "libnanyc-config.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


void Scope::emitDebugpos(AST::Node& node) {
	if (node.offset > 0) {
		auto it = context.offsetToLine.lower_bound(node.offset);
		if (it != context.offsetToLine.end()) {
			bool emit = (it->first == node.offset);
			if (not emit and context.offsetToLine.begin() != it)
				emit = (--it != context.offsetToLine.end());
			if (emit)
				addDebugCurrentPosition(it->second, node.offset - it->first);
		}
	}
}


void Scope::doEmitTmplParameters() {
	assert(lastPushedTmplParams.get());
	if (not lastPushedTmplParams->empty()) {
		auto& outIR = sequence();
		for (auto& pair : *lastPushedTmplParams) {
			if (pair.second.empty())
				ir::emit::tpush(outIR, pair.first);
			else
				ir::emit::tpush(outIR, pair.first, pair.second);
		}
	}
	lastPushedTmplParams = nullptr;
}


AnyString Scope::getSymbolNameFromASTNode(AST::Node& node) {
	assert(node.rule == AST::rgSymbolName);
	assert(node.children.size() == 1);
	auto& identifier = node.children.front();
	if (unlikely(identifier.rule != AST::rgIdentifier)) {
		unexpectedNode(node, "expected identifier");
		return AnyString{};
	}
	if (unlikely(identifier.text.size() > Config::maxSymbolNameLength)) {
		auto err = error(node) << "identifier name too long";
		err.message.origins.location.pos.offsetEnd = err.message.origins.location.pos.offset + identifier.text.size();
		return AnyString{};
	}
	return identifier.text;
}


void Scope::checkForUnknownAttributes() const {
	assert(!!attributes);
	if (unlikely(not attributes->flags.empty())) {
		if (unlikely(context.ignoreAtoms))
			return;
		auto& attrs = *attributes;
		auto& node  = attrs.node;
		if (unlikely(attrs.flags(Attributes::Flag::pushSynthetic)))
			error(node, "invalid use of expr attribute '__nanyc_synthetic'");
		if (attrs.flags(Attributes::Flag::shortcircuit))
			error(node, "invalid use of func attribute 'shortcircuit'");
		if (attrs.flags(Attributes::Flag::doNotSuggest))
			error(node, "invalid use of func attribute 'nosuggest'");
		if (attrs.flags(Attributes::Flag::builtinAlias))
			error(node, "invalid use of func attribute 'builtinalias'");
		if (attrs.flags(Attributes::Flag::threadproc))
			error(node, "invalid use of func attribute 'thread'");
	}
}


} // namespace Producer
} // namespace ir
} // namespace ny
