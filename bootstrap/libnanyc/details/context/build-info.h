#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/grammar/nany.h"
#include "details/ast/tree-index.h"
#include "details/intrinsic/catalog.h"
#include "details/ir/sequence.h"
#include "nany/nany.h"



namespace ny {


class BuildInfoSource final {
public:
	BuildInfoSource(nybuild_cf_t& cf)
		: cf(cf) {
	}

public:
	nybuild_cf_t& cf;

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

		//! Parse / IR result
		bool success = false;
	}
	parsing;

}; // class BuildInfo




} // namespace ny
