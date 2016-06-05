#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "libnany-config.h"
#include "details/utils/check-for-valid-identifier-name.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{

	namespace // anonymous
	{

		class ClassInspector final
		{
		public:
			typedef CString<Config::maxSymbolNameLength, false> ClassnameType;

		public:
			ClassInspector(Scope& scope);

			bool inspect(const AST::Node& node);


		public:
			//! Parent scope
			Scope& scope;
			//! Class body
			const AST::Node* body = nullptr;

			//! name of the class (if any)
			ClassnameType classname;


		private:
			bool inspectClassname(const AST::Node&);
			void createDefaultOperators();

		}; // class ClassInspector





		inline ClassInspector::ClassInspector(Scope& scope)
			: scope(scope)
		{
		}


		inline bool ClassInspector::inspectClassname(const AST::Node& node)
		{
			classname = scope.getSymbolNameFromASTNode(node);
			return not classname.empty()
				and checkForValidIdentifierName(scope.report(), node, classname, false, true);
		}


		inline bool ClassInspector::inspect(const AST::Node& node)
		{
			// exit status
			bool success = true;

			for (auto& childptr: node.children)
			{
				auto& child = *childptr;
				switch (child.rule)
				{
					case AST::rgSymbolName:
					{
						success &= inspectClassname(child);
						break;
					}
					case AST::rgClassBody:
					{
						body = &child;
						break;
					}
					case AST::rgVisibility:
					{
						/*currently ignore */ break;
					}
					case AST::rgClassTemplateParams:
					{
						success &= scope.visitASTDeclGenericTypeParameters(child);
						break;
					}
					default:
						success &= scope.ICEUnexpectedNode(child, "[class]");
				}
			}

			if (classname.empty())
				classname = "<anonymous>";

			// default implemented functions
			// those functions are currently empty, but may be filled later at instanciation
			// if the equivalent operator 'new', 'clone' & 'dispose' are not defined
			// by the user, those functions will be renamed on the fly

			auto& reuse = scope.context.reuse;

			if (!reuse.operatorDefault.node)
				scope.context.prepareReuseForClasses();

			reuse.operatorDefault.funcname->text = "^default-new";
			success &= scope.visitASTFunc(*reuse.operatorDefault.node);

			reuse.operatorDefault.funcname->text = "^obj-dispose";
			success &= scope.visitASTFunc(*reuse.operatorDefault.node);

			reuse.operatorClone.funcname->text = "^obj-clone";
			success &= scope.visitASTFunc(*(reuse.operatorClone.node));
			return success;
		}



	} // anonymous namespace









	bool Scope::visitASTClass(const AST::Node& node, LVID* localvar)
	{
		assert(node.rule == AST::rgClass);
		assert(not node.children.empty());

		uint32_t lvid = 0;
		if (localvar) // create the lvid before the new scope
			*localvar = (lvid = nextvar());

		// new scope
		IR::Producer::Scope scope{*this};
		scope.moveAttributes(*this);
		// reset internal counter for generating local classdef in the current scope
		scope.resetLocalCounters();
		scope.kind = Scope::Kind::kclass;
		scope.broadcastNextVarID = false;


		auto& out = sequence();

		// creating a new blueprint for the function
		uint32_t bpoffset = out.emitBlueprintClass(lvid);
		uint32_t bpoffsiz = out.emitBlueprintSize();
		uint32_t bpoffsck = out.emitStackSizeIncrease();

		// making sure that debug info are available
		scope.addDebugCurrentFilename();
		scope.emitDebugpos(node);

		bool success = true;

		// evaluate the whole function, and grab the node body for continuing evaluation
		auto* body = ([&]() -> const AST::Node*
		{
			ClassInspector inspector{scope};
			success = inspector.inspect(node);

			auto& operands = out.at<ISA::Op::blueprint>(bpoffset);
			operands.name = out.stringrefs.ref(inspector.classname);
			return inspector.body;
		})();

		if (likely(body != nullptr))
		{
			if (debugmode)
				scope.comment("\nclass body"); // comment for clarity in code

			// continue evaluating the func body independantly of the previous data and results
			for (auto& stmtnode: body->children)
				success &= scope.visitASTStmt(*stmtnode);
		}


		// end of the blueprint
		out.emitEnd();
		uint32_t blpsize = out.opcodeCount() - bpoffset;
		out.at<ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		out.at<ISA::Op::stacksize>(bpoffsck).add = scope.pNextVarID + 1u;
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
