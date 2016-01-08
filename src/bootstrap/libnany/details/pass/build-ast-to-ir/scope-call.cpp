#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{

	inline bool Scope::visitASTExprCallParameters(const Node& node, uint32_t shortcircuitlabel)
	{
		assert(node.rule == rgCall);
		// parameter index
		uint paramCount = 0;
		// at least one named-parameter has been encountered ?
		bool namedParameterFound = false;

		// information related to all pushed parameters
		struct {
			LVID localvar = 0;
			AnyString name; // acquired named
		}
		pushedIndexedParam[Config::maxPushedParameters];

		// the program
		auto& out = program();


		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgCallParameter:
				case rgCallNamedParameter:
				{
					if (unlikely(paramCount >= Config::maxPushedParameters))
					{
						error(child) << "too many pushed parameters";
						return false;
					}

					// is the parameter named ?
					bool isNamed = (child.rule == rgCallNamedParameter);
					// detect mixed-up named and indexed parameters
					if (not isNamed)
					{
						if (unlikely(namedParameterFound))
							return (error(node) << "got mixed indexed and named parameters");
					}
					else
						namedParameterFound = true;


					// get the temporary parameter info
					auto& paraminfo = pushedIndexedParam[paramCount];
					// parameter name
					AnyString name;

					for (auto& paramChildptr: child.children)
					{
						auto& paramchild = *paramChildptr;
						switch (paramchild.rule)
						{
							case rgExpr:
							{
								// this func call may be used by some boolean operators so we may
								// want to evaluate the second argument only if the first argument
								// does not suffice to determine the value of the expression
								//
								// the code to execute can not be decided yet (needs typing) so
								// some 'nop' will be inserted to avoid unwanted code/offset shifts
								if (0 != shortcircuitlabel)
								{
									if (paramCount == 1) // 2nd argument
									{
										// insert _BEFORE_ the second parameter

										// remember the current offset for inserting code later if needed
										// (+1 to put the offset after 'stackalloc')
										out.reserve(out.opcodeCount() + 4);

										// this temporary variable is exactly 'label id + 1' and already reserved
										// (see below) and can be used for reading a variable member
										out.emitStackalloc(shortcircuitlabel + 1, nyt_any);
										out.emitraw<IR::ISA::Op::nop>();
										out.emitraw<IR::ISA::Op::nop>();
									}

									// generate scope to prevent against unwanted var release in case of jump
									out.emitScope();
								}

								bool visited = visitASTExpr(paramchild, paraminfo.localvar);
								if (unlikely(not visited))
									return false;

								if (unlikely(isNamed and name.empty()))
								{
									ICE(paramchild) << "got an empty name for a named parameter";
									return false;
								}
								break;
							}
							case rgIdentifier:
							{
								if (likely(isNamed))
								{
									name = paramchild.text;
									paraminfo.name = acquireString(name);

									assert(shortcircuitlabel == 0 and "named param not accepted with shortcircuit");
									break;
								}
								// no break here - to go to unexecped node
							}
							default:
								return ICEUnexpectedNode(paramchild, "[ir/expr/call-parameter]");
						}
					}

					++paramCount;
					break;
				}

				//case rgCallNamedParameter:
				default:
					return ICEUnexpectedNode(child, "[ir/expr/call]");
			}
		} // each child


		if (0 != shortcircuitlabel)
			out.emitPragmaShortcircuitMetadata(shortcircuitlabel);

		// push all parameters, indexed and named
		for (uint i = 0; i != paramCount; ++i)
		{
			const auto& info = pushedIndexedParam[i];
			if (info.name.empty())
				out.emitPush(info.localvar);
			else
				out.emitPush(info.localvar, info.name);
		}
		return true;
	}



	bool Scope::visitASTExprCall(const Node* node, LVID& localvar, const Node* parent)
	{
		assert(!node or node->rule == rgCall);

		emitDebugpos(node);

		auto& out = program();
		// ask to resolve the call to operator ()
		auto func = out.emitStackalloc(nextvar(), nyt_any);
		out.emitIdentify(func, "^()", localvar);

		auto callret = out.emitStackalloc(nextvar(), nyt_any);
		localvar = callret; // the new expression value


		if (!node or node->children.empty())
		{
			// no pushed parameter - direct call - no need for scopes or something else
			out.emitCall(callret, func);
			return true;
		}
		else
		{
			// short-circuit only applies to func call with 2 parameters
			// but the code for minimal evaluation can not be determined yet (some
			// member may have to be read, or maybe it should not be done at all)

			// this flag will only prepare some room for additional opcodes if required
			bool shortcircuit = (parent != nullptr and parent->rule == rgIdentifier)
				and (node->children.size() == 2)
				and (parent->text == "^and" or parent->text == "^or");

			if (not shortcircuit)
			{
				IR::OpcodeScopeLocker opscope{out};
				bool success = visitASTExprCallParameters(*node);
				emitDebugpos(*node);
				out.emitCall(callret, func);
				return success;
			}
			else
			{
				// instanciate a label for potential jumps (minimal evaluation)
				uint32_t shortcircuitlabel = nextvar();
				// instanciate a temporary variable, if needed for reading a variable
				// member (from the class 'bool' for example)
				// (thus the lvid for this will be exactly (shortcircuitlabel + 1))
				nextvar();

				// since a shortcircuit label is provided, 2 scopes (because 2 parameters)
				// must be generated by `visitASTExprCallParameters`

				bool success = visitASTExprCallParameters(*node, shortcircuitlabel);
				emitDebugpos(*node);
				out.emitCall(callret, func);

				// end of scope for 2nd parameter
				// this way, a jump just after the first one will not try to release the second one
				out.emitEnd();

				out.emitLabel(shortcircuitlabel);
				out.emitEnd();
				return success;
			}
			return false;
		}
	}







} // namespace Producer
} // namespace IR
} // namespace Nany
