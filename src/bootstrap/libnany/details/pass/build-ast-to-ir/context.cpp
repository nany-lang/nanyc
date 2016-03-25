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

		cache.node = new AST::Node{AST::rgNew};
		AST::Node::Ptr typeDecl = new AST::Node{AST::rgTypeDecl};
		cache.node->children.push_back(typeDecl);
		cache.classname = new AST::Node{AST::rgIdentifier};
		typeDecl->children.push_back(cache.classname);

		AST::Node::Ptr call = new AST::Node{AST::rgCall};
		cache.node->children.push_back(call);
		AST::Node::Ptr callParam = new AST::Node{AST::rgCallParameter};
		call->children.push_back(callParam);
		AST::Node::Ptr expr = new AST::Node{AST::rgExpr};
		callParam->children.push_back(expr);
		cache.lvidnode = new AST::Node{AST::rgRegister};
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
		reuse.closure.node = new AST::Node{AST::rgExpr};

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

		AST::Node::Ptr exprValue = new AST::Node{AST::rgExprValue};
		reuse.closure.node->children.push_back(exprValue);
		AST::Node::Ptr exprValueNew = new AST::Node{AST::rgNew};
		exprValue->children.push_back(exprValueNew);

		AST::Node::Ptr nnew = new AST::Node{AST::rgNew};
		exprValueNew->children.push_back(nnew);
		AST::Node::Ptr typedecl = new AST::Node{AST::rgTypeDecl};
		nnew->children.push_back(typedecl);
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
