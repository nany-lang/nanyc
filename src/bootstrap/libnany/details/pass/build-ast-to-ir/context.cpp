#include "context.h"
#include "details/ast/ast.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	void Context::useNamespace(const AnyString& nmspc)
	{
		if (not nmspc.empty())
		{
			nmspc.words(".", [&](const AnyString& part) -> bool
			{
				sequence.emitNamespace(part);
				return true;
			});
		}
	}


	void Context::generateLineIndexes(const AnyString& content)
	{
		uint line = 1;
		uint length = content.size();
		for (uint i = 0; i != length; ++i)
		{
			if (content[i] == '\n')
				offsetToLine.emplace(i, ++line);
		}
	}


	void Context::prepareReuseForLiterals()
	{
		// new (+2)
		//     type-decl
		//     |   identifier: i64
		//     call (+3)
		//         call-parameter
		//             expr
		//                 register: <lvid>

		auto& cache = reuse.literal;

		cache.node = new Node{rgNew};
		Node::Ptr typeDecl = new Node{rgTypeDecl};
		cache.node->children.push_back(typeDecl);
		cache.classname = new Node{rgIdentifier};
		typeDecl->children.push_back(cache.classname);

		Node::Ptr call = new Node{rgCall};
		cache.node->children.push_back(call);
		Node::Ptr callParam = new Node{rgCallParameter};
		call->children.push_back(callParam);
		Node::Ptr expr = new Node{rgExpr};
		callParam->children.push_back(expr);
		cache.lvidnode = new Node{rgRegister};
		expr->children.push_back(cache.lvidnode);
	}


	void Context::prepareReuseForClasses()
	{
		reuse.operatorDefault.node
			= AST::createNodeFunc(reuse.operatorDefault.funcname);

		reuse.operatorClone.node
			= AST::createNodeFuncCrefParam(reuse.operatorClone.funcname, "rhs");
	}


	void Context::prepareReuseForClosures()
	{
		reuse.closure.node = new Node{rgExpr};

		// expr
		//     expr-value
		//         new
		//             type-decl
		//                 class
		//                     class-body
		//                         expr
		//                         |   expr-value
		//                         |       function (+2)
		//                         |           function-kind
		//                         |           |   function-kind-operator (+2)
		//                         |           |       tk-operator
		//                         |           |       function-kind-opname: ()
		//                         |           func-body
		//                         |               return-inline (+3)
		//                         |                   tk-arrow: ->
		//                         |                   expr
		//                         |                   |   ...

		Node::Ptr exprValue = new Node{rgExprValue};
		reuse.closure.node->children.push_back(exprValue);
		Node::Ptr exprValueNew = new Node{rgNew};
		exprValue->children.push_back(exprValueNew);

		Node::Ptr nnew = new Node{rgNew};
		exprValueNew->children.push_back(nnew);
		Node::Ptr typedecl = new Node{rgTypeDecl};
		nnew->children.push_back(typedecl);
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
