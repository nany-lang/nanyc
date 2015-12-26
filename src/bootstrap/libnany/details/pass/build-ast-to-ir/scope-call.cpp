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

	inline bool Scope::visitASTExprCallParameters(const Node& node)
	{
		assert(node.rule == rgCall);
		// parameter name
		AnyString name;
		// parameter index
		uint paramCount = 0;
		// at least one named-parameter has been encountered ?
		bool namedParameterFound = false;

		// information related to all pushed parameters
		struct ParamInfo final
		{
			LVID localvar = 0;
			const char* name = nullptr; // acquired named
		};

		// indexed parameters
		ParamInfo pushedIndexedParam[Config::maxPushedParameters];

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgCallParameter:
				case rgCallNamedParameter:
				{
					if (paramCount >= Config::maxPushedParameters)
					{
						error(child) << "too many pushed parameters";
						break;
					}

					// get the temporary parameter info
					auto& paraminfo = pushedIndexedParam[paramCount];

					// is the parameter named ?
					bool isNamed = (child.rule == rgCallNamedParameter);
					if (not isNamed)
					{
						if (unlikely(namedParameterFound))
						{
							error(node) << "got mixed indexed and named parameters";
							return false;
						}
					}
					else
						namedParameterFound = true;

					for (auto& paramChildptr: child.children)
					{
						auto& paramchild = *paramChildptr;
						switch (paramchild.rule)
						{
							case rgExpr:
							{
								bool visited = visitASTExpr(paramchild, paraminfo.localvar);
								if (unlikely(not visited))
									return false;

								if (isNamed and (unlikely(name.empty())))
									ICE(paramchild) << "got an empty name for a named parameter";
								break;
							}
							case rgIdentifier:
							{
								if (likely(isNamed))
								{
									name = paramchild.text;
									paraminfo.name = acquireString(name).c_str();
									break;
								}
								// no break here - to go to unexecped node
							}
							default:
								return ICEUnexpectedNode(child, "[ir/expr/call-parameter]");
						}
					}

					++paramCount;
					break;
				}

				//case rgCallNamedParameter:

				default:
					return ICEUnexpectedNode(child, "[ir/expr/call]");
			}

			if (paramCount >= Config::maxPushedParameters)
				break;
		}


		// push all parameters, indexed and named
		for (uint i = 0; i != paramCount; ++i)
		{
			const auto& info = pushedIndexedParam[i];
			if (info.name == nullptr)
				program().emitPush(info.localvar);
			else
				program().emitPush(info.localvar, info.name);
		}

		return true;
	}



	bool Scope::visitASTExprCall(const Node* node, LVID& localvar)
	{
		assert(!node or node->rule == rgCall);

		emitDebugpos(node);

		// ask to resolve the call to operator ()
		auto func = program().emitStackalloc(nextvar(), nyt_any);
		program().emitIdentify(func, "^()", localvar);

		auto callret = program().emitStackalloc(nextvar(), nyt_any);
		localvar = callret; // the new expression value


		if (!node or node->children.empty()) // no pushed parameter - direct call
		{
			program().emitCall(callret, func);
			return true;
		}
		else
		{
			// some parameters must be pushed
			OpcodeScopeLocker opscope{program()};
			// visit all parameters
			bool success = visitASTExprCallParameters(*node);
			emitDebugpos(node);
			program().emitCall(callret, func);
			return success;
		}
	}







} // namespace Producer
} // namespace IR
} // namespace Nany
