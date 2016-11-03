#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprCallParameters(AST::Node& node, ShortcircuitUpdate* shortcircuit)
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

		for (auto& child: node.children)
		{
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
					AnyString name; // parameter name

					for (auto& paramchild: child.children)
					{
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
								if (shortcircuit != nullptr)
								{
									if (paramCount == 1) // insert 'nop' _BEFORE_ the second parameter
									{
										// remember the current offset for inserting code later if needed
										// (+1 to put the offset after 'stackalloc')
										out.reserve(out.opcodeCount() + 4);
										// this temporary variable is exactly 'label id + 1' and already reserved
										// (see visitASTExprCall) and can be used for reading a variable member
										shortcircuit->offsetStackalloc = out.opcodeCount();
										out.emitStackalloc(0 /*label + 1*/, nyt_any);
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
									return (ice(paramchild) << "got an empty name for a named parameter");
								break;
							}
							case AST::rgIdentifier:
							{
								if (likely(isNamed))
								{
									name = paramchild.text;
									paraminfo.name = acquireString(name);
									assert(shortcircuit == nullptr and "named param not accepted with shortcircuit");
									break;
								}
								// no break here - to go to unexecped node
							}
							default:
								return unexpectedNode(paramchild, "[ir/expr/call-parameter]");
						}
					}
					++paramCount;
					break;
				}
				//case AST::rgCallNamedParameter:
				default:
					return unexpectedNode(child, "[ir/expr/call]");
			}
		} // each child

		if (shortcircuit)
		{
			shortcircuit->offsetPragma = out.opcodeCount();
			out.emitPragmaShortcircuitMetadata(0 /*label*/);
			if (unlikely(paramCount != 2))
				ice(node) << "invalid number of parameters for shortcircuit";
		}

		// push all parameters, indexed and named
		for (uint32_t i = 0; i != paramCount; ++i)
		{
			const auto& info = pushedIndexedParam[i];
			if (info.name.empty())
				out.emitPush(info.localvar);
			else
				out.emitPush(info.localvar, info.name);
		}
		return true;
	}


	bool Scope::visitASTExprCall(AST::Node* node, LVID& localvar, AST::Node* parent)
	{
		assert(!node or node->rule == AST::rgCall);

		emitDebugpos(node ? node : parent);
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

				// using another intermediate value to mutate it into a real object
				// if necessary
				uint32_t callret = out.emitStackalloc(nextvar(), nyt_any);
				localvar = callret; // the new expression value
				// since a shortcircuit label is provided, 2 scopes (because 2 parameters)
				// must be generated by `visitASTExprCallParameters`

				// for post labelid update
				ShortcircuitUpdate scupdt;

				bool success = visitASTExprCallParameters(*node, &scupdt);
				if (unlikely(!!lastPushedTmplParams))
				{
					lastPushedTmplParams = nullptr;
					ice(*node) << "unsupported generic type parameters for shortcircuit functions";
					success = false;
				}
				emitDebugpos(*node);
				out.emitCall(ret__bool, func);
				// end of scope for 2nd parameter
				// this way, a jump just after the first one will not try to release the second one
				out.emitEnd();

				// Shortcircuit label
				uint32_t sclabel = out.emitLabel(nextvar());
				// instanciate a temporary variable, if needed for reading a variable
				// member (from the class 'bool' for example)
				// (thus the lvid for this will be exactly (shortcircuitlabel + 1))
				nextvar(); // sclabel + 1
				if (scupdt.offsetPragma != 0)
					out.at<IR::ISA::Op::pragma>(scupdt.offsetPragma).value.shortcircuitMetadata.label = sclabel;
				if (scupdt.offsetStackalloc != 0)
					out.at<IR::ISA::Op::stackalloc>(scupdt.offsetStackalloc).lvid = sclabel + 1;

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
