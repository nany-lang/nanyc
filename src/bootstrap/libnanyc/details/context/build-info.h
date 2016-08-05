#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/grammar/nany.h"
#include "details/ast/tree-index.h"
#include "details/intrinsic/intrinsic-table.h"
#include "details/ir/sequence.h"
#include "nany/nany.h"



namespace Nany
{


	class BuildInfoSource final
	{
	public:
		BuildInfoSource(nybuild_cf_t& cf)
			: cf(cf)
		{}

		size_t inspectMemoryUsage() const;

	public:
		nybuild_cf_t& cf;

		struct
		{
			//! Nany parser for the current content
			AST::Parser parser;
			//! Namespace of the file
			std::pair<YString, AST::Node*> nmspc;
			//! AST manipulation
			ASTHelper ast;
			//! Root node
			AST::Node::Ptr rootnode;

			//! The original sequence, generated from the normalized AST
			IR::Sequence sequence;

			//! Parse / IR result
			bool success = false;
		}
		parsing;

	}; // class BuildInfo




} // namespace Nany
