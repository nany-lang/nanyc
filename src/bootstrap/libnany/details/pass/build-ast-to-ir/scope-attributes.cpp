#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include "libnany-config.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTAttributes(const AST::Node& node)
	{
		assert(node.rule == AST::rgAttributes);

		if (unlikely(node.children.empty()))
			return true;

		// instanciate attributes
		pAttributes = std::make_unique<Attributes>(node);
		auto& attrs = *pAttributes;

		// attribute name
		ShortString32 attrname;
		// temporary buffer for value interpretation
		ShortString32 value;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			// checking for node type
			if (unlikely(child.rule != AST::rgAttributesParameter))
				return ICEUnexpectedNode(child, "invalid node, not attribute parameter");

			AST::Node* nodevalue;
			switch (child.children.size())
			{
				case 1: nodevalue = nullptr; break;
				case 2: nodevalue = AST::Node::Ptr::WeakPointer(child.children[1]); break;
				default:
					return ICEUnexpectedNode(child, "invalid attribute parameter node");
			}
			AST::Node& nodekey = *child.children[0];
			if (unlikely(nodekey.rule != AST::rgEntity))
				return ICEUnexpectedNode(child, "invalid attribute parameter name type");
			if (nodevalue)
			{
				if (unlikely(nodevalue->rule != AST::rgEntity))
					return (error(child) << "unsupported expression for attribute value");
			}
			attrname.clear();
			value.clear();
			if (not AST::retrieveEntityString(attrname, nodekey))
				return ICEUnexpectedNode(child, "invalid entity");

			switch (attrname[0])
			{
				case 'b':
				{
					if (attrname == "builtinalias")
					{
						if (unlikely(!nodevalue))
							return (error(child) << "value expected for attribute '" << attrname << '\'');

						attrs.builtinAlias = nodevalue;
						attrs.flags += Attributes::Flag::builtinAlias;
						break;
					}
				}

				// [[fallthru]]
				case 's':
				{
					if (attrname == "shortcircuit")
					{
						if (unlikely(!nodevalue))
							return (error(child) << "value expected for attribute '" << attrname << '\'');

						AST::retrieveEntityString(value, *nodevalue);
						bool isTrue = (value == "__true");
						if (not isTrue and (value.empty() or value != "__false"))
						{
							error(*child.children[1]) << "invalid shortcircuit value, expected '__false' or '__true', got '"
								<< value << "'";
							return false;
						}

						if (isTrue)
							attrs.flags += Attributes::Flag::shortcircuit;
						break;
					}
					if (attrname == "suggest")
					{
						if (unlikely(!nodevalue))
							return (error(child) << "value expected for attribute '" << attrname << '\'');

						AST::retrieveEntityString(value, *nodevalue);
						bool onoff = true;
						if (value.empty() or not value.to(onoff))
							return (error(child) << "invalid attribute 'suggest' value, expected 'false' or 'true'");

						attrs.flags += Attributes::Flag::doNotSuggest;
						break;
					}
				}

				// [[fallthru]]
				case '_':
				{
					if (attrname == "__synthetic")
					{
						attrs.flags += Attributes::Flag::pushSynthetic;
						if (unlikely(nodevalue))
							return (error(child) << "the attribute '__synthetic' does not accept values");
						break;
					}
				}

				// [[fallthru]]
				default:
				{
					error(child) << "unknown attribute '" << attrname << '\'';
					return false;
				}
			}
		}
		return true;
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
