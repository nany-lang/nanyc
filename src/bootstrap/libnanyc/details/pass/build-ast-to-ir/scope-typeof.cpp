#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprTypeof(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgTypeof);
		localvar = 0;
		using TypeInfo = std::pair<std::reference_wrapper<AST::Node>, uint32_t>;
		std::vector<TypeInfo> types;

		for (auto& child: node.children)
		{
			if (child.rule == AST::rgCall)
			{
				types.reserve(child.children.size());
				for (auto& param: child.children)
				{
					if (param.rule == AST::rgCallParameter)
						types.emplace_back(std::make_pair(std::ref(param.firstChild()), 0u));
				}
			}
		}

		uint32_t count = static_cast<uint32_t>(types.size());
		if (unlikely(count == 0))
			return (error(node) << "at least one type is required for 'typeof'");

		auto& out = sequence();
		bool success = true;

		IR::Producer::Scope scope{*this};
		for (auto& typeinfo: types)
		{
			auto& expr = typeinfo.first.get();
			emitDebugpos(expr);
			OpcodeCodegenDisabler codegen{out};
			if (debugmode)
				out.emitComment("typeof expression");
			success &= scope.visitASTExpr(expr, typeinfo.second);
		}
		if (count > 1u and success)
		{
			uint32_t previous = 0;
			for (auto& typeinfo: types)
			{
				out.emitCommonType(typeinfo.second, previous);
				previous = typeinfo.second;
			}
		}
		localvar = types.back().second;
		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
