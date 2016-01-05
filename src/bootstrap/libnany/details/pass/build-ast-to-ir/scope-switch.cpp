#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include <iostream>

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprSwitch(Node& node)
	{
		assert(node.rule == rgSwitch);
		bool success = true;

		if (debugmode)
			program().emitComment("switch");

		auto& out = program();
		OpcodeScopeLocker opscopeSwitch{out};

		// the variable id of the initial condition
		uint32_t valuelvid = 0;

		// a temporary variable to compute if a 'case' value matches or not
		// this variable is reused for each 'case'
		uint32_t casecondlvid = nextvar();
		out.emitStackalloc(casecondlvid, nyt_bool);


		// the current implementation generates a 'if' statement for each 'case'
		// these variables are for simulating an AST node
		Node::Ptr exprCase = new Node{rgExpr};
		Node::Ptr cond = AST::createNodeIdentifier("^==");
		exprCase->children.push_back(cond);
		Node::Ptr call = new Node{rgCall};
		cond->children.push_back(call);

		// lhs
		Node::Ptr lhs = new Node{rgCallParameter};
		call->children.push_back(lhs);
		ShortString16 lvidstr;
		Node::Ptr lhsExpr = new Node{rgExpr};
		lhs->children.push_back(lhsExpr);
		Node::Ptr lhsValue = new Node{rgRegister};
		lhsExpr->children.push_back(lhsValue);

		Node::Ptr rhs = new Node{rgCallParameter};
		call->children.push_back(rhs);


		// using a scope for the body to have proper variable scoping
		Node bodyScope{rgScope};

		//! list of labels to update (to jump at the end of the switch-case when a cond matches)
		std::vector<uint32_t> labels;
		labels.reserve(node.children.size());


		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case rgSwitchCase:
				{
					if (debugmode)
						out.emitComment("case");
					if (unlikely(valuelvid == 0))
						return (ICE(child) << "switch: unexpected lvid value");
					if (unlikely(child.children.size() != 2))
						return ICEUnexpectedNode(child, "[ir/switch/case]");

					if (success)
					{
						OpcodeScopeLocker opscopeCase{out};
						rhs->children.clear();
						rhs->children.push_back(child.children[0]);

						bodyScope.children.clear();
						bodyScope.children.push_back(child.children[1]);

						labels.emplace_back();
						success &= generateIfStmt(*exprCase, bodyScope, /*else*/nullptr, &(labels.back()));
					}
					break;
				}

				case rgSwitchExpr:
				{
					if (child.children.size() == 1)
					{
						auto& condition = *(child.children[0]);
						success &= visitASTExpr(condition, valuelvid, false);

						// updating lhs for operator ==
						lvidstr = valuelvid;
						lhsValue->text = lvidstr;
						break;
					}
					// do not break
				}
				default:
					return ICEUnexpectedNode(child, "[ir/switch]");
			}
		}

		uint32_t labelEnd = nextvar();
		out.emitLabel(labelEnd);

		// update all labels for jumping to the end
		for (auto& offset: labels)
			out.at<IR::ISA::Op::jmp>(offset).label = labelEnd;

		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
