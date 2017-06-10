#pragma once
#include "details/compiler/compiler.h"

namespace ny {
namespace compiler {

namespace {

bool makeASTFromSource(ny::compiler::Source& source) {
	auto& parser = source.parsing.parser;
	bool loaded = (source.content.empty())
		? parser.loadFromFile(source.filename)
		: parser.load(source.content);
	return loaded and parser.root;
}

} // namespace

} // namespace compiler
} // namespace ny
