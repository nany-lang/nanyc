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
		{}


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
					default:
						success &= scope.ICEUnexpectedNode(child, "[class]");
				}
			}

			// default implemented functions
			// those functions are currently empty, but may be filled later at instanciation
			// if the equivalent operator 'new', 'clone' & 'dispose' are not defined
			// by the user, those functions will be renamed on the fly
			Node::Ptr defaultNew = AST::createNodeFunc("^default-new");
			success &= scope.visitASTFunc(*defaultNew);

			Node::Ptr defaultDispose = AST::createNodeFunc("^obj-dispose");
			success &= scope.visitASTFunc(*defaultDispose);

			Node::Ptr defaultClone = AST::createNodeFuncCrefParam("^obj-clone", "rhs");
			success &= scope.visitASTFunc(*defaultClone);
			return success;
		}



	} // anonymous namespace









	bool Scope::visitASTClass(Node& node)
	{
		assert(node.rule == rgClass);
		assert(not node.children.empty());

		// new scope
		IR::Producer::Scope scope{*this};
		// reset internal counter for generating local classdef in the current scope
		scope.resetLocalCounters();
		scope.kind = Scope::Kind::kclass;
		scope.broadcastNextVarID = false;

		// creating a new blueprint for the function
		uint32_t bpoffset = program().emitBlueprintClass();
		uint32_t bpoffsiz = program().emitBlueprintSize();
		uint32_t bpoffsck = program().emitStackSizeIncrease();

		// making sure that debug info are available
		scope.addDebugCurrentFilename();
		scope.emitDebugpos(node);

		bool success = true;

		// evaluate the whole function, and grab the node body for continuing evaluation
		Node* body = ([&]() -> Node*
		{
			ClassInspector inspector{scope};
			success = inspector.inspect(node);

			auto& operands = program().at<ISA::Op::pragma>(bpoffset);
			operands.value.blueprint.name = program().stringrefs.ref(inspector.classname);
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
		program().emitEnd();
		uint32_t blpsize = program().opcodeCount() - bpoffset;
		program().at<ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		program().at<ISA::Op::stacksize>(bpoffsck).add = scope.pNextVarID + 1;
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
