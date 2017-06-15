#pragma once
#include <nanyc/program.h>
#include "details/reporting/message.h"
#include "details/grammar/nany.h"
#include "details/ast/tree-index.h"
#include "details/ir/sequence.h"
#include "details/atom/classdef-table.h"
#include "details/intrinsic/catalog.h"
#include <memory>
#include <cassert>

namespace ny {
namespace compiler {

struct Source final {
	struct {
		//! ny parser for the current content
		AST::Parser parser;
		//! Namespace of the file
		std::pair<YString, AST::Node*> nmspc;
		//! AST manipulation
		ASTHelper ast;
		//! Root node
		yuni::Ref<AST::Node> rootnode;
		//! The original sequence, generated from the normalized AST
		ir::Sequence ircode;
	}
	parsing;

	AnyString content;
	AnyString filename;
	yuni::String storageContent;  // only used when owning the data
	yuni::String storageFilename; // (same)

	ir::Sequence& sequence() { return parsing.ircode; }
};

struct Compdb final {
	Compdb(const nycompile_opts_t& opts): opts(opts) {}

	yuni::Mutex mutex;
	ClassdefTable cdeftable;
	intrinsic::Catalog intrinsics;
	const nycompile_opts_t& opts;
	Logs::Message messages{Logs::Level::none};
	struct final {
		uint32_t count = 0;
		std::unique_ptr<Source[]> items;
		Source& operator [] (uint32_t i) { assert(i < count); return items[i]; }
	}
	sources;
};

} // namespace compiler
} // namespace ny
