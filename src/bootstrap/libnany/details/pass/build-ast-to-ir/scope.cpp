#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::ICEUnexpectedNode(const AST::Node& node, const AnyString& location) const
	{
		if (not AST::ruleIsError(node.rule))
		{
			auto report = context.report.ICE();
			auto rulename = AST::ruleToString(node.rule);
			report << "unexpected node '" << rulename << '\'';
			if (not location.empty())
				report << ' ' << location;

			setErrorFrom(report, node);
		}
		return false;
	}


	Logs::Report Scope::error(const AST::Node& node)
	{
		auto err = context.report.error();
		setErrorFrom(err, node);
		return err;
	}


	Logs::Report Scope::warning(const AST::Node& node)
	{
		auto err = context.report.warning();
		setErrorFrom(err, node);
		return err;
	}


	Logs::Report Scope::ICE(const AST::Node& node) const
	{
		auto ice = context.report.ICE();
		setErrorFrom(ice, node);
		return ice;
	}



	void Scope::fetchLineAndOffsetFromNode(const AST::Node& node, yuint32& line, yuint32& offset) const
	{
		if (node.offset > 0)
		{
			auto it = context.offsetToLine.lower_bound(node.offset);
			if (it != context.offsetToLine.end())
			{
				if (it->first == node.offset or (--it != context.offsetToLine.end()))
				{
					line = it->second;
					offset = node.offset - it->first;
					return;
				}
			}
			line = 1;
			offset = 1;
		}
		else
		{
			line = 0;
			offset = 0;
		}
	}


	void Scope::emitDebugpos(const AST::Node& node)
	{
		if (likely(node.offset > 0))
		{
			auto it = context.offsetToLine.lower_bound(node.offset);
			if (it != context.offsetToLine.end())
			{
				if (it->first == node.offset or (--it != context.offsetToLine.end()))
					addDebugCurrentPosition(it->second, node.offset - it->first);
			}
		}
	}


	AnyString Scope::getSymbolNameFromASTNode(const AST::Node& node)
	{
		assert(node.rule == AST::rgSymbolName);
		assert(node.children.size() == 1);

		auto& identifier = *(node.children[0]);
		if (unlikely(identifier.rule != AST::rgIdentifier))
		{
			ICEUnexpectedNode(node, "expected identifier");
			return AnyString{};
		}

		if (unlikely(identifier.text.size() > Config::maxSymbolNameLength))
		{
			auto err = report().error() << "identifier name too long";
			err.message.origins.location.pos.offsetEnd = err.message.origins.location.pos.offset + identifier.text.size();
			return AnyString{};
		}

		return identifier.text;
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
