#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include "details/ir/scope-locker.h"
#include <limits>

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{

	bool Scope::visitASTExprStringLiteral(const AST::Node& node, LVID& localvar)
	{
		// when called, this rule represents an internal cstring
		// thus, this function is not called by an user-defined string
		emitDebugpos(node);
		localvar = sequence().emitStackallocText(nextvar(), node.text);
		return true;
	}


	bool Scope::visitASTExprString(const AST::Node& node, yuint32& localvar)
	{
		assert(node.rule == AST::rgString);

		emitDebugpos(node);

		// transform string literals into implicit `new string()`
		//
		// several patterns can arise for declaring a string:
		// 1. an empty string: like `var s = ""`
		// 2. a string with a simple literal: `var s = "hello world"`
		// 3. or a more complex string, with string interpolation for example
		//   `var w = "world"; var s = "hello \(w) another literal";` (where \(w) is an expr)

		// create a string object
		if (!context.reuse.string.createObject)
		{
			// new (+2)
			//     type-decl
			//     |   identifier: string
			auto& cache = context.reuse.string;
			cache.createObject = new AST::Node{AST::rgNew};
			AST::Node::Ptr typeDecl = new AST::Node{AST::rgTypeDecl};
			cache.createObject->children.push_back(typeDecl);
			AST::Node::Ptr classname = new AST::Node{AST::rgIdentifier};
			typeDecl->children.push_back(classname);
			classname->text = "string";
		}
		bool success = visitASTExprNew(*(context.reuse.string.createObject), localvar);
		if (unlikely(not success))
			return false;

		// sequence output
		auto& out = sequence();

		uint32_t idlvid = out.emitStackalloc(nextvar(), nyt_any);
		out.emitIdentify(idlvid, "append", localvar);
		uint32_t calllvid = out.emitStackalloc(nextvar(), nyt_any);
		out.emitIdentify(calllvid, "^()", idlvid); // functor

		context.reuse.string.text.clear();
		AST::Node* firstLiteralNode = nullptr;

		auto flush = [&]() {
			emitDebugpos(*firstLiteralNode);
			uint32_t sid = out.emitStackallocText(nextvar(), context.reuse.string.text);
			uint32_t lid = out.emitStackalloc_u64(nextvar(), nyt_u64, context.reuse.string.text.size());
			uint32_t ret = out.emitStackalloc(nextvar(), nyt_void);
			out.emitPush(sid); // text: __text
			out.emitPush(lid); // size: __u64
			out.emitCall(ret, calllvid);
		};


		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgStringLiteral:
				{
					if (not child.text.empty())
					{
						context.reuse.string.text += child.text;
						if (nullptr == firstLiteralNode)
							firstLiteralNode = &child;
					}
					break;
				}

				case AST::rgStringInterpolation:
				{
					if (child.children.empty())
						break;

					if (not context.reuse.string.text.empty())
					{
						flush();
						firstLiteralNode = nullptr;
						context.reuse.string.text.clear();
					}

					for (auto& ptr: child.children)
					{
						auto& expr = *ptr;
						if (unlikely(expr.rule != AST::rgExpr))
							return unexpectedNode(expr, "[string-interpolation]");

						// creating a scope for temporary expression (string interpolation)
						IR::OpcodeScopeLocker opscope{out};
						emitDebugpos(expr);
						uint32_t lvid;
						if (not visitASTExpr(expr, lvid))
							return false;

						emitDebugpos(expr);
						uint32_t ret = out.emitStackalloc(nextvar(), nyt_void);
						out.emitPush(lvid);
						out.emitCall(ret, calllvid);
					}
					break;
				}

				case AST::rgCharExtended:
				{
					if (nullptr == firstLiteralNode)
						firstLiteralNode = &child;

					if (child.text.size() == 1)
					{
						switch (child.text[0])
						{
							case 'r':  context.reuse.string.text += '\r'; break;
							case 'n':  context.reuse.string.text += '\n'; break;
							case 't':  context.reuse.string.text += '\t'; break;
							case 'v':  context.reuse.string.text += '\v'; break;
							case '\\': context.reuse.string.text += '\\'; break;
							case '"':  context.reuse.string.text += '"'; break;
							case 'a':  context.reuse.string.text += '\a'; break;
							case 'b':  context.reuse.string.text += '\b'; break;
							case 'f':  context.reuse.string.text += '\f'; break;
							case 'c':  /* produce no further output */ break;
							default: error(child) << "invalid escaped character '\\" << child.text << '\''; return false;
						}
					}
					else
					{
						error(child) << "invalid escaped character '\\" << child.text << '\'';
						return false;
					}
					break;
				}

				default:
				{
					return unexpectedNode(*childptr, "[expr-string]");
				}
			}
		}

		if (firstLiteralNode != nullptr)
			flush();
		return true;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
