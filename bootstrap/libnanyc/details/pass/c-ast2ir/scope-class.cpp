#include <yuni/yuni.h>
#include "scope.h"
#include "details/utils/check-for-valid-identifier-name.h"

using namespace Yuni;





namespace ny
{
namespace ir
{
namespace Producer
{

	namespace {


	struct ClassInspector final
	{
		using ClassnameType = CString<Config::maxSymbolNameLength, false>;

		ClassInspector(Scope& parentscope, uint32_t lvid);

		bool inspect(AST::Node& node);
		bool inspectBody(AST::Node& node);

		//! Parent scope
		ir::Producer::Scope scope;

		uint32_t lvid = 0;
		uint32_t bpoffset;
		uint32_t bpoffsiz;
		uint32_t bpoffsck;

		//! Class body
		AST::Node* body = nullptr;
		//! name of the class (if any)
		ClassnameType classname;

	private:
		bool inspectClassname(AST::Node&);
		void createDefaultOperators();

	}; // class ClassInspector




	ClassInspector::ClassInspector(Scope& parentscope, uint32_t lvid)
		: scope{parentscope}
		, lvid{lvid}
	{
		// new scope
		scope.moveAttributes(parentscope);
		// reset internal counter for generating local classdef in the current scope
		scope.resetLocalCounters();
		scope.kind = Scope::Kind::kclass;
		scope.broadcastNextVarID = false;
		// a log of code relies on the fact that %{atomid:1} is the type of the return value
		// starting from +2 to avoid to always check for the type of the atom everywhere in the code
		scope.nextvar();

		auto& out = scope.sequence();
		// creating a new blueprint for the function
		bpoffset = out.emitBlueprintClass(lvid);
		bpoffsiz = out.emitBlueprintSize();
		bpoffsck = ir::emit::increaseStacksize(out);
		// making sure that debug info are available
		scope.context.invalidateLastDebugLine();
		ir::emit::dbginfo::filename(out, scope.context.dbgSourceFilename);
	}


	bool ClassInspector::inspectClassname(AST::Node& node)
	{
		classname = scope.getSymbolNameFromASTNode(node);
		return not classname.empty()
			and checkForValidIdentifierName(node, classname, IdNameFlag::isType);
	}


	bool ClassInspector::inspect(AST::Node& node)
	{
		scope.emitDebugpos(node);
		// exit status
		bool success = true;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgSymbolName:
					success &= inspectClassname(child);
					break;
				case AST::rgClassBody:
					body = &child;
					break;
				case AST::rgVisibility:
					/*currently ignore */ break;
				case AST::rgClassTemplateParams:
					success &= scope.visitASTDeclGenericTypeParameters(child);
					break;
				default:
					success &= unexpectedNode(child, "[class]");
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

		scope.emitDebugpos(node);
		// default constructor
		reuse.operatorDefault.funcname->text = "^default-new";
		success &= scope.visitASTFunc(*reuse.operatorDefault.node);
		// destructor
		reuse.operatorDefault.funcname->text = "^dispose";
		success &= scope.visitASTFunc(*reuse.operatorDefault.node);
		// copy constructor
		reuse.operatorClone.funcname->text = "^obj-clone";
		success &= scope.visitASTFunc(*(reuse.operatorClone.node));
		return success;
	}


	bool ClassInspector::inspectBody(AST::Node& node)
	{
		bool success = true;
		auto& out = scope.sequence();
		// evaluate the whole function, and grab the node body for continuing evaluation
		{
			success = inspect(node);
			auto& operands = out.at<ISA::Op::blueprint>(bpoffset);
			operands.name = out.stringrefs.ref(classname);
		}
		if (likely(body != nullptr))
		{
			ir::emit::trace(out, "\nclass body");
			// continue evaluating the func body independantly of the previous data and results
			for (auto& stmtnode: body->children)
				success &= scope.visitASTStmt(stmtnode);
		}

		ir::emit::scopeEnd(out);
		uint32_t blpsize = out.opcodeCount() - bpoffset;
		out.at<ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		out.at<ISA::Op::stacksize>(bpoffsck).add = scope.nextVarID + 1u;
		return success;
	}


	} // anonymous namespace




	bool Scope::visitASTClass(AST::Node& node, LVID* localvar)
	{
		assert(node.rule == AST::rgClass);
		assert(not node.children.empty());
		if (unlikely(context.ignoreAtoms))
			return true;

		uint32_t lvid = 0;
		if (localvar) // create the lvid before the new scope
			*localvar = (lvid = nextvar());
		auto classbuilder = std::make_unique<ClassInspector>(*this, lvid);
		return classbuilder->inspectBody(node);
	}




} // namespace Producer
} // namespace ir
} // namespace ny
