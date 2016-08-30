#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprSwitch(AST::Node& node)
	{
		assert(node.rule == AST::rgSwitch);
		bool success = true;

		if (debugmode)
			sequence().emitComment("switch");

		auto& out = sequence();
		OpcodeScopeLocker opscopeSwitch{out};
		emitDebugpos(node);

		// the variable id of the initial condition
		uint32_t valuelvid = 0;

		// a temporary variable to compute if a 'case' value matches or not
		// this variable is reused for each 'case'
		uint32_t casecondlvid = nextvar();
		out.emitStackalloc(casecondlvid, nyt_bool);


		// the current implementation generates a 'if' statement for each 'case'
		// these variables are for simulating an AST node
		AST::Node::Ptr exprCase = new AST::Node{AST::rgExpr};
		AST::Node::Ptr cond = AST::createNodeIdentifier("^==");
		exprCase->children.push_back(cond);
		AST::Node::Ptr call = new AST::Node{AST::rgCall};
		cond->children.push_back(call);

		// lhs
		AST::Node::Ptr lhs = new AST::Node{AST::rgCallParameter};
		call->children.push_back(lhs);
		ShortString16 lvidstr;
		AST::Node::Ptr lhsExpr = new AST::Node{AST::rgExpr};
		lhs->children.push_back(lhsExpr);
		AST::Node::Ptr lhsValue = new AST::Node{AST::rgRegister};
		lhsExpr->children.push_back(lhsValue);

		AST::Node::Ptr rhs = new AST::Node{AST::rgCallParameter};
		call->children.push_back(rhs);


		// using a scope for the body to have proper variable scoping
		AST::Node bodyScope{AST::rgScope};

		//! list of labels to update (to jump at the end of the switch-case when a cond matches)
		auto labels = std::make_unique<uint32_t[]>(node.children.size());
		uint32_t labelCount = 0;


		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgSwitchCase:
				{
					if (debugmode)
						out.emitComment("case");
					if (unlikely(valuelvid == 0))
						return (ice(child) << "switch: unexpected lvid value");
					if (unlikely(child.children.size() != 2))
						return unexpectedNode(child, "[ir/switch/case]");

					if (success)
					{
						OpcodeScopeLocker opscopeCase{out};
						rhs->children.clear();
						rhs->children.push_back(&(child.children[0]));

						bodyScope.children.clear();
						bodyScope.children.push_back(&(child.children[1]));

						success &= generateIfStmt(*exprCase, bodyScope, /*else*/nullptr, &(labels[labelCount]));
						++labelCount;
					}
					break;
				}

				case AST::rgSwitchExpr:
				{
					if (child.children.size() == 1)
					{
						auto& condition = child.children[0];
						emitDebugpos(condition);
						success &= visitASTExpr(condition, valuelvid, false);

						// updating lhs for operator ==
						lvidstr = valuelvid;
						lhsValue->text = lvidstr;
						break;
					}
					// do not break
				}
				default:
					return unexpectedNode(child, "[ir/switch]");
			}
		}

		emitDebugpos(node);
		uint32_t labelEnd = out.emitLabel(nextvar());

		// update all labels for jumping to the end
		for (uint32_t i = 0 ; i != labelCount; ++i)
			out.at<IR::ISA::Op::jmp>(labels[i]).label = labelEnd;

		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
