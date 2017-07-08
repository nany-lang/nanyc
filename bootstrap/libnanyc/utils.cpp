#include <nanyc/utils.h>
#include "libnanyc.h"
#include <yuni/core/string.h>
#include <yuni/io/file.h>
#include "details/grammar/nany.h"

namespace {

void visitNode(nyconsole_t& console, const ny::AST::Node& node, bool hasSibling, yuni::String& indent, yuni::String& tmp) {
	auto print = [&](const AnyString& text) {
		if (not text.empty())
			console.write(&console, text.c_str(), text.size());
	};
	auto setcolor = [&](nycolor_t color) {
		console.set_color(&console, color);
	};
	setcolor(nyc_blue);
	print(indent);
	if (unlikely(ny::AST::ruleIsError(node.rule))) {
		setcolor(nyc_red);
		print(ruleToString(node.rule));
		setcolor(nyc_default);
	}
	else {
		if (unlikely(node.hasAttributeImportant())) {
			setcolor(nyc_magenta);
			print(ruleToString(node.rule));
			setcolor(nyc_default);
		}
		else {
			setcolor(nyc_default);
			print(ruleToString(node.rule));
		}
	}
	if (node.hasAttributeCapture()) {
		console.write(&console, ": ", 2);
		setcolor(nyc_green);
		tmp = node.text;
		tmp.replace("\n", "\\n");
		tmp.replace("\t", "\\t");
		tmp.replace("\r", "\\r");
		print(tmp);
		setcolor(nyc_default);
	}
	else {
		// it can be interresting to print the text itself when the node
		// is a simple text capture
		const AnyString& textCapture = node.attributeCapturedText();
		if (not textCapture.empty()) {
			console.write(&console, ", ", 2);
			setcolor(nyc_green);
			print(textCapture);
			setcolor(nyc_default);
		}
	}
	if (node.children.size() > 1) {
		setcolor(nyc_blue);
		yuni::ShortString16 xx;
		xx << " (+" << node.children.size() << ')';
		print(xx);
		setcolor(nyc_default);
	}
	console.write(&console, "\n", 1);
	if (not node.children.empty()) {
		if (hasSibling)
			indent.append("|   ", 4);
		else
			indent.append("    ", 4);
		for (uint32_t i = 0; i != node.children.size(); ++i) {
			bool hasSibling = (i != node.children.size() - 1);
			visitNode(console, node.children[i], hasSibling, indent, tmp);
		}
		indent.chop(4);
	}
}

bool tryFindErrorNode(const ny::AST::Node& allnodes) {
	std::vector<std::reference_wrapper<const ny::AST::Node>> stack;
	stack.reserve(128); // to reduce memory reallocations
	stack.push_back(std::cref(allnodes));
	do {
		auto& node = stack.back().get();
		if (ny::AST::ruleIsError(node.rule))
			return false;
		stack.pop_back();
		uint32_t i = node.children.size();
		while (i-- > 0)
			stack.push_back(std::cref(node.children[i]));
	}
	while (not stack.empty());
	return true;
}

} // namespace

nybool_t nyast_print_content(nyconsole_t* console, const char* content, size_t len) {
	if (!console)
		return nyfalse;
	if (!len)
		return nytrue;
	if (len > 1024 * 1024 * 1024 or !content)
		return nyfalse;
	try {
		ny::AST::Parser parser;
		bool ok = parser.load(AnyString{content, static_cast<uint32_t>(len)});
		if (unlikely(not ok or !parser.root))
			return nyfalse;
		yuni::String tmp;
		yuni::String indent;
		visitNode(*console, *parser.root, false, indent, tmp);
		return nytrue;
	}
	catch (...) {
	}
	return nyfalse;
}

nybool_t nyast_print_file(nyconsole_t* console, const char* filename, size_t len) {
	if (!console)
		return nyfalse;
	if (!len)
		return nytrue;
	if (len > 1024 * 1024 * 1024 or !filename)
		return nyfalse;
	try {
		yuni::Clob content;
		auto err = yuni::IO::File::LoadFromFile(content, AnyString{filename, static_cast<uint32_t>(len)});
		if (unlikely(err != yuni::IO::errNone))
			return nyfalse;
		return nyast_print_content(console, content.c_str(), content.size());
	}
	catch (...) {
	}
	return nyfalse;
}

nybool_t nyparse_check_content(const char* content, size_t len) {
	if (!len)
		return nytrue;
	if (len > 1024 * 1024 * 1024 or !content)
		return nyfalse;
	try {
		ny::AST::Parser parser;
		bool success = parser.load(AnyString{content, static_cast<uint32_t>(len)})
			and (parser.root != nullptr) and tryFindErrorNode(*(parser.root));
		if (success)
			return nytrue;
	}
	catch (...) {
	}
	return nyfalse;
}

nybool_t nyparse_check_file(const char* filename, size_t len) {
	if (!len)
		return nytrue;
	if (len > 1024 * 1024 * 1024 or !filename)
		return nyfalse;
	try {
		yuni::Clob content;
		auto err = yuni::IO::File::LoadFromFile(content, AnyString{filename, static_cast<uint32_t>(len)});
		if (unlikely(err != yuni::IO::errNone))
			return nyfalse;
		return nyparse_check_content(content.c_str(), content.size());
	}
	catch (...) {
	}
	return nyfalse;
}
