#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "libnany-config.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/ast/ast.h"

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

			bool inspect(Node& node);


		public:
			//! Parent scope
			Scope& scope;
			//! Class body
			Node* body = nullptr;

			//! name of the class (if any)
			ClassnameType classname;


		private:
			bool inspectClassname(Node&);
			void createDefaultOperators();

		}; // class ClassInspector





		inline ClassInspector::ClassInspector(Scope& scope)
			: scope(scope)
		{
		}


		inline bool ClassInspector::inspectClassname(Node& node)
		{
			classname = scope.getSymbolNameFromASTNode(node);
			return not classname.empty();
		}


		inline bool ClassInspector::inspect(Node& node)
		{
			// exit status
			bool success = true;

			for (auto& childptr: node.children)
			{
				auto& child = *childptr;
				switch (child.rule)
				{
					case rgSymbolName:  { success &= inspectClassname(child); break; }
					case rgClassBody:   { body = &child; break; }
					case rgVisibility:  { /*currently ignore */ break; }
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
				reuse.operatorDefault.node = AST::createNodeFunc(reuse.operatorDefault.funcname);

			if (!reuse.operatorClone.node)
				reuse.operatorClone.node = AST::createNodeFuncCrefParam(reuse.operatorClone.funcname, "rhs");

			reuse.operatorDefault.funcname->text = "^default-new";
			success &= scope.visitASTFunc(*reuse.operatorDefault.node);

			reuse.operatorDefault.funcname->text = "^obj-dispose";
			success &= scope.visitASTFunc(*reuse.operatorDefault.node);

			reuse.operatorClone.funcname->text = "^obj-clone";
			success &= scope.visitASTFunc(*(reuse.operatorClone.node));
			return success;
		}



	} // anonymous namespace









	bool Scope::visitASTClass(Node& node, LVID* /*localvar*/)
	{
		assert(node.rule == rgClass);
		assert(not node.children.empty());

		// new scope
		IR::Producer::Scope scope{*this};
		// reset internal counter for generating local classdef in the current scope
		scope.resetLocalCounters();
		scope.kind = Scope::Kind::kclass;
		scope.broadcastNextVarID = false;

		auto& out = sequence();

		// creating a new blueprint for the function
		uint32_t bpoffset = out.emitBlueprintClass();
		uint32_t bpoffsiz = out.emitBlueprintSize();
		uint32_t bpoffsck = out.emitStackSizeIncrease();

		// making sure that debug info are available
		scope.addDebugCurrentFilename();
		scope.emitDebugpos(node);

		bool success = true;

		// evaluate the whole function, and grab the node body for continuing evaluation
		Node* body = ([&]() -> Node*
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
		out.at<ISA::Op::stacksize>(bpoffsck).add = scope.pNextVarID + 1;
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
