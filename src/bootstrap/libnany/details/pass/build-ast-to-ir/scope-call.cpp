#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "libnany-config.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{

	inline bool Scope::visitASTExprCallParameters(const AST::Node& node, uint32_t shortcircuitlabel)
	{
		assert(node.rule == AST::rgCall);
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

		// the sequence
		auto& out = sequence();

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgCallParameter:
				case AST::rgCallNamedParameter:
				{
					if (unlikely(paramCount >= Config::maxPushedParameters))
					{
						error(child) << "too many pushed parameters";
						return false;
					}

					// is the parameter named ?
					bool isNamed = (child.rule == AST::rgCallNamedParameter);
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
							case AST::rgExpr:
							{
								// ** short circuit evaluation ** The second parameter may be evaluated
								// only if the first one does not suffice to determine the value of
								// the expression
								//
								// Nothing can be decided yet (needs typing) so a few 'nop' will be
								// added to avoid invalid pointers (to no longer matching instructions) and to
								// reduce costly memory operations for inserting instructions
								if (0 != shortcircuitlabel)
								{
									if (paramCount == 1) // insert 'nop' _BEFORE_ the second parameter
									{
										// remember the current offset for inserting code later if needed
										// (+1 to put the offset after 'stackalloc')
										out.reserve(out.opcodeCount() + 4);
										// this temporary variable is exactly 'label id + 1' and already reserved
										// (see visitASTExprCall) and can be used for reading a variable member
										out.emitStackalloc(shortcircuitlabel + 1, nyt_any);
										out.emitraw<IR::ISA::Op::nop>(); // fieldget, if not builtin type
										out.emitraw<IR::ISA::Op::nop>(); // jump
									}

									// generate scope to prevent against unwanted var release in case of jump
									// (will be released later in `Scope::visitASTExprCall`)
									out.emitScope();
								}

								// visit the parameter
								bool visited = visitASTExpr(paramchild, paraminfo.localvar);
								if (unlikely(not visited))
									return false;

								if (unlikely(isNamed and name.empty()))
									return (ICE(paramchild) << "got an empty name for a named parameter");
								break;
							}
							case AST::rgIdentifier:
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

				//case AST::rgCallNamedParameter:
				default:
					return ICEUnexpectedNode(child, "[ir/expr/call]");
			}
		} // each child


		if (0 != shortcircuitlabel)
		{
			out.emitPragmaShortcircuitMetadata(shortcircuitlabel);
		}

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



	bool Scope::visitASTExprCall(const AST::Node* node, LVID& localvar, const AST::Node* parent)
	{
		assert(!node or node->rule == AST::rgCall);

		emitDebugpos(node);

		auto& out = sequence();
		// ask to resolve the call to operator ()
		auto func = out.emitStackalloc(nextvar(), nyt_any);
		out.emitIdentify(func, "^()", localvar);

		if (!node or node->children.empty())
		{
			auto callret = out.emitStackalloc(nextvar(), nyt_any);
			localvar = callret; // the new expression value

			// no pushed parameter - direct call - no need for scopes or something else
			// ... but template parameters if any
			emitTmplParametersIfAny();
			out.emitCall(callret, func);
			return true;
		}
		else
		{
			// short-circuit only applies to func call with 2 parameters
			// but the code for minimal evaluation can not be determined yet (some
			// member may have to be read, or maybe it should not be done at all)

			// this flag will only prepare some room for additional opcodes if required
			bool shortcircuit = (parent != nullptr and parent->rule == AST::rgIdentifier)
				and (node->children.size() == 2)
				and (parent->text == "^and" or parent->text == "^or");

			if (not shortcircuit)
			{
				auto callret = out.emitStackalloc(nextvar(), nyt_any);
				localvar = callret; // the new expression value

				IR::OpcodeScopeLocker opscope{out};
				bool success = visitASTExprCallParameters(*node);
				emitTmplParametersIfAny();
				emitDebugpos(*node);
				out.emitCall(callret, func);
				return success;
			}
			else
			{
				uint32_t ret__bool = out.emitStackalloc(nextvar(), nyt_any);

				// instanciate a label for potential jumps (minimal evaluation)
				uint32_t shortcircuitlabel = nextvar();

				// instanciate a temporary variable, if needed for reading a variable
				// member (from the class 'bool' for example)
				// (thus the lvid for this will be exactly (shortcircuitlabel + 1))
				nextvar();

				// using another intermediate value to mutate it into a real object
				// if necessary
				uint32_t callret = out.emitStackalloc(nextvar(), nyt_any);
				localvar = callret; // the new expression value

				// since a shortcircuit label is provided, 2 scopes (because 2 parameters)
				// must be generated by `visitASTExprCallParameters`

				bool success = visitASTExprCallParameters(*node, shortcircuitlabel);

				if (unlikely(not lastPushedTmplParams.empty()))
				{
					lastPushedTmplParams.clear();
					ICE(*node) << "unsupported generic type parameters for shortcircuit functions";
					success = false;
				}

				emitDebugpos(*node);
				out.emitCall(ret__bool, func);

				// end of scope for 2nd parameter
				// this way, a jump just after the first one will not try to release the second one
				out.emitEnd();

				out.emitLabel(shortcircuitlabel);
				out.emitEnd();

				// reserving variable for sizeof
				out.emitStackalloc(nextvar(), nyt_u64);
				out.emitPragmaShortcircuitMutateToBool(callret, ret__bool);
				return success;
			}
		}
		return false;
	}







} // namespace Producer
} // namespace IR
} // namespace Nany
