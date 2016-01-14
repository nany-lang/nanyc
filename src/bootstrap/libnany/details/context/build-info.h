#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/grammar/nany.h"
#include "details/ast/tree-index.h"
#include "details/intrinsic/intrinsic-table.h"
#include "details/ir/program.h"
#include "isolate.h"
#include "nany/nany.h"



namespace Nany
{

	class BuildInfoContext final : public Yuni::Policy::ObjectLevelLockable<BuildInfoContext>
	{
	public:
		BuildInfoContext(nycontext_t& context)
			: isolate(context)
		{}

	public:
		Isolate isolate;

		//! the global status flag - must be protected by mutex when accessed from sources
		bool success = true;
		//! timestamp (in ms) of the start of current build
		yint64 buildtime = 0;
		//! list of all sources currently built (keep program for each source)
		std::vector<Source::Ptr> sources;
	};



	class BuildInfoSource final
	{
	public:
		size_t inspectMemoryUsage() const;

	public:
		struct
		{
			//! Nany parser for the current content
			Nany::Parser parser;
			//! Namespace of the file
			std::pair<YString, Nany::Node*> nmspc;
			//! AST manipulation
			ASTHelper ast;
			//! Root node
			Node::Ptr rootnode;

			//! The original program, generated from the normalized AST
			IR::Program program;

			//! Parse / IR result
			bool success = false;
		}
		parsing;

	}; // class BuildInfo




} // namespace Nany
