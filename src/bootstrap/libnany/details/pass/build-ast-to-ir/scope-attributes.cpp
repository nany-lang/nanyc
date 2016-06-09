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
				case 'n':
				{
					if (attrname == "nodiscard")
					{
						if (unlikely(nodevalue))
							return (error(child) << "no value expected for attribute '" << attrname << '\'');
						break;
					}
				}

				// [[fallthru]]
				case 'p':
				{
					if (attrname == "per")
					{
						if (unlikely(!nodevalue))
							return (error(child) << "value expected for attribute '" << attrname << '\'');

						AST::retrieveEntityString(value, *nodevalue);
						if (value == "thread")
						{
							warning(child) << "ignored attribute 'per: thread'";
						}
						else if (value == "process")
						{
							warning(child) << "ignored attribute 'per: process'";
						}
						else
							return (error(child) << "invalid 'per' value");
						break;
					}
				}

				// [[fallthru]]
				case 's':
				{
					if (attrname == "nosuggest")
					{
						attrs.flags += Attributes::Flag::doNotSuggest;
						if (unlikely(nodevalue))
							return (error(child) << "the attribute '" << attrname << "' does not accept values");
						break;
					}
				}

				// [[fallthru]]
				case '_':
				{
					if (attrname == "__nanyc_builtinalias")
					{
						if (unlikely(!nodevalue))
							return (error(child) << "value expected for attribute '" << attrname << '\'');

						attrs.builtinAlias = nodevalue;
						attrs.flags += Attributes::Flag::builtinAlias;
						break;
					}

					if (attrname == "__nanyc_shortcircuit")
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

					if (attrname == "__nanyc_synthetic")
					{
						attrs.flags += Attributes::Flag::pushSynthetic;
						if (unlikely(nodevalue))
							return (error(child) << "the attribute '" << attrname << "' does not accept values");
						break;
					}

					// [FALLBACK]
					// ignore vendor specific attributes (starting by '__')
					if (attrname.size() > 2 and attrname[1] == '_')
					{
						// emit a warning for the unsupported specific nany attributes
						if (unlikely(attrname.startsWith("__nanyc_")))
							warning(child) << "unknown nanyc attribute '" << attrname << "'";
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
