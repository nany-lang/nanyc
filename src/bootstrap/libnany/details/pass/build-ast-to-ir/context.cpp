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
		//	 type-decl
		//	 |   identifier: i64
		//	 call (+3)
		//		 call-parameter
		//			 expr
		//				 register: <lvid>

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


	void Context::prepareReuseForVariableMembers()
	{
		reuse.func.node = AST::createNodeFunc(reuse.func.funcname);

		AST::Node::Ptr funcBody = new AST::Node{AST::rgFuncBody};
		(reuse.func.node)->children.push_back(funcBody);

		AST::Node::Ptr expr = new AST::Node{AST::rgExpr};
		funcBody->children.push_back(expr);

		// intrinsic (+2)
		//	   entity (+3)
		//	   |   identifier: nanyc
		//	   |   identifier: fieldset
		//	   call (+7)
		//		   call-parameter
		//		   |   expr
		//		   |	   <expr A>
		//		   call-parameter
		//		   |   expr
		//		   |	   <expr B>
		AST::Node::Ptr intrinsic = new AST::Node{AST::rgIntrinsic};
		expr->children.push_back(intrinsic);
		intrinsic->children.push_back(AST::createNodeIdentifier("^fieldset"));

		AST::Node::Ptr call = new AST::Node{AST::rgCall};
		intrinsic->children.push_back(call);

		// param 2 - expr
		{
			reuse.func.callparam = new AST::Node{AST::rgCallParameter};
			call->children.push_back(reuse.func.callparam);
		}
		// param text varname
		{
			AST::Node::Ptr callparam = new AST::Node{AST::rgCallParameter};
			call->children.push_back(callparam);
			AST::Node::Ptr pexpr = new AST::Node{AST::rgExpr};
			callparam->children.push_back(pexpr);

			reuse.func.varname = new AST::Node{AST::rgStringLiteral};
			pexpr->children.push_back(reuse.func.varname);
		}
	}


	void Context::prepareReuseForClosures()
	{
		reuse.closure.node = new AST::Node{AST::rgExpr};

		// expr
		//	 expr-value
		//		 new
		//			 type-decl
		//				 class
		//					 class-body
		//						 expr
		//						 |   expr-value
		//						 |	   function (+2)
		//						 |		   function-kind
		//						 |		   |   function-kind-operator (+2)
		//						 |		   |	   function-kind-opname: ()
		//						 |		   func-body
		//						 |			   return-inline (+3)
		//						 |				   expr
		//						 |				   |   ...

		auto& exprValue = reuse.closure.node->append(AST::rgExprValue);
		auto& nnew	    = exprValue.append(AST::rgNew);
		auto& typedecl  = nnew.append(AST::rgTypeDecl);
		auto& nclass	= typedecl.append(AST::rgClass);
		auto& cbody	    = nclass.append(AST::rgClassBody);
		auto& bodyExpr  = cbody.append(AST::rgExpr);
		auto& bodyValue = bodyExpr.append(AST::rgExprValue);

		auto& func = bodyValue.append(AST::rgFunction);

		auto& funcname =
			func.append(AST::rgFunctionKind, AST::rgFunctionKindOperator, AST::rgFunctionKindOpname);
		funcname.text = "()";

		auto& params = func.append(AST::rgFuncParam);
		reuse.closure.params = &params;

		auto& rettype = func.append(AST::rgFuncReturnType);
		reuse.closure.rettype = &rettype;

		auto& funcBody = func.append(AST::rgFuncBody);
		reuse.closure.funcbody = &funcBody;
	}


	void Context::prepareReuseForIn()
	{
		// expr
		//   expr-value (+2)
		//       expr-group
		//       |   expr-value
		//       |       identifier: expr
		//       expr-sub-dot
		//           identifier: makeview
		//               call
		//                   call-parameter
		//                       expr
		//                           expr-value
		//                               function (+4)
		//                                   function-kind
		//                                   |   function-kind-function
		//                                   func-params
		//                                   |   func-param (+2)
		//                                   |       cref
		//                                   |       identifier: i
		//                                   func-return-type
		//                                   |   type
		//                                   |       type-qualifier
		//                                   |           ref
		//                                   func-body
		//                                       return
		//                                           expr
		//                                               expr-value
		//                                                   identifier: predicate
		//
		reuse.inset.node = new AST::Node{AST::rgExpr};
		auto& exprValue = reuse.inset.node->append(AST::rgExprValue);

		auto& exprGroup = exprValue.append(AST::rgExprGroup);
		reuse.inset.container = &exprGroup;

		auto& viewname = exprValue.append(AST::rgExprSubDot, AST::rgIdentifier);
		reuse.inset.viewname = &viewname;

		auto& call = viewname.append(AST::rgCall);
		auto& paramExprValue = call.append(AST::rgCallParameter, AST::rgExpr, AST::rgExprValue);
		auto& func = paramExprValue.append(AST::rgFunction);
		func.append(AST::rgFunctionKind, AST::rgFunctionKindFunction);

		auto& funcparams = func.append(AST::rgFuncParams);
		auto& funcparam = funcparams.append(AST::rgFuncParam);
		funcparam.append(AST::rgCref);
		auto& cursorname = funcparam.append(AST::rgIdentifier);
		reuse.inset.elementname = &cursorname;

		auto& qualifiers = func.append(AST::rgFuncReturnType, AST::rgType, AST::rgTypeQualifier);
		qualifiers.append(AST::rgRef);

		auto& funcbody = func.append(AST::rgFuncBody);
		auto& ret = funcbody.append(AST::rgReturn);
		reuse.inset.predicate = &ret;

		reuse.inset.premadeAlwaysTrue = new AST::Node{AST::rgExpr};
		auto& alwaysTrue = reuse.inset.premadeAlwaysTrue->append(AST::rgExprValue, AST::rgIdentifier);
		alwaysTrue.text = "true";
	}



	void Context::prepareReuseForLoops()
	{
		// scope
		//     expr
		//         expr-value
		//             if (+2)
		//                 expr
		//                 |   identifier: ^not
		//                 |       call
		//                 |           call-parameter
		//                 |               expr
		//                 |                   expr-value
		//                 |                       identifier: myview
		//                 |                           expr-sub-dot
		//                 |                               identifier: empty
		//                 |                                   call
		//                 if-then
		//                     expr
		//                         expr-value
		//                             scope (+2)
		//                                 var (+3)
		//                                 |   ref
		//                                 |   identifier: cursor
		//                                 |   var-assign (+2)
		//                                 |       operator-kind
		//                                 |       |   operator-assignment: =
		//                                 |       expr
		//                                 |           expr-value
		//                                 |               identifier: myview
		//                                 |                   expr-sub-dot
		//                                 |                       identifier: cursor
		//                                 |                           call
		//                                 expr
		//                                     expr-value
		//                                         if (+2)
		//                                             expr
		//                                             |   expr-value
		//                                             |       identifier: cursor
		//                                             |           expr-sub-dot
		//                                             |               identifier: findfirst
		//                                             |                   call
		//                                             if-then
		//                                                 expr
		//                                                     expr-value
		//                                                         scope
		//                                                             do-while (+2)
		//                                                                 expr
		//                                                                 |   expr-value
		//                                                                 |       scope (+2)
		//                                                                 |           var (+3)
		//                                                                 |           |   ref
		//                                                                 |           |   identifier: i
		//                                                                 |           |   var-assign (+2)
		//                                                                 |           |       operator-kind
		//                                                                 |           |       |   operator-assignment: =
		//                                                                 |           |       expr
		//                                                                 |           |           expr-value
		//                                                                 |           |               identifier: cursor
		//                                                                 |           |                   expr-sub-dot
		//                                                                 |           |                       identifier: get
		//                                                                 |           |                           call
		//                                                                 |           expr
		//                                                                 |               expr-value
		//                                                                 |                   scope
		//                                                                 |                       <.. user code here ..>
		//                                                                 expr
		//                                                                     expr-value
		//                                                                         identifier: cursor
		//                                                                             expr-sub-dot
		//                                                                                 identifier: next
		//                                                                                     call

		reuse.loops.node = new AST::Node{AST::rgExpr};
		auto& nif = reuse.loops.node->append(AST::rgExprValue, AST::rgIf);

		// condition if, "if empty"
		{
			auto& identifier = nif.append(AST::rgExpr, AST::rgIdentifier);
			identifier.text = "^not";
			auto& param = identifier.append(AST::rgCall, AST::rgCallParameter);
			auto& viewname = param.append(AST::rgExpr, AST::rgExprValue, AST::rgRegister);
			reuse.loops.viewlvid[0] = &viewname;
			auto& empty = viewname.append(AST::rgExprSubDot, AST::rgIdentifier);
			empty.text = "empty";
			empty.append(AST::rgCall);
		}

		// if not empty, then...
		auto& ifthen = nif.append(AST::rgIfThen);
		auto& scope = ifthen.append(AST::rgExpr, AST::rgExprValue, AST::rgScope);

		// capture the cursor
		{
			auto& var = scope.append(AST::rgVar);
			var.append(AST::rgRef);
			reuse.loops.cursorname[0] = &var.append(AST::rgIdentifier);
			auto& varAssign = var.append(AST::rgVarAssign);
			auto& opassign = varAssign.append(AST::rgOperatorKind, AST::rgOperatorAssignment);
			opassign.text = "=";

			auto& value = varAssign.append(AST::rgExpr, AST::rgExprValue, AST::rgRegister);
			reuse.loops.viewlvid[1] = &value;
			auto& cursorCall = value.append(AST::rgExprSubDot, AST::rgIdentifier);
			cursorCall.text = "cursor";
			cursorCall.append(AST::rgCall);
		}

		auto& ifFindFirst = scope.append(AST::rgExpr, AST::rgExprValue, AST::rgIf);
		// if findFirst condition
		{
			auto& value = ifFindFirst.append(AST::rgExpr, AST::rgExprValue, AST::rgIdentifier);
			reuse.loops.cursorname[1] = &value;
			auto& cursorCall = value.append(AST::rgExprSubDot, AST::rgIdentifier);
			cursorCall.text = "findFirst";
			cursorCall.append(AST::rgCall);
		}
		// .. then
		auto& beforescope = ifFindFirst.append(AST::rgIfThen);
		auto& scopeThen = beforescope.append(AST::rgExpr, AST::rgExprValue, AST::rgScope);

		auto& dowhile = scopeThen.append(AST::rgDoWhile);
		auto& scopeFor = dowhile.append(AST::rgExpr, AST::rgExprValue, AST::rgScope);

		auto& varElement = scopeFor.append(AST::rgVar);
		varElement.append(AST::rgRef);
		reuse.loops.elementname = &varElement.append(AST::rgIdentifier);
		auto& varGet = varElement.append(AST::rgVarAssign);
		auto& assign = varGet.append(AST::rgOperatorKind, AST::rgOperatorAssignment);
		assign.text = "=";
		auto& cursorGet = varGet.append(AST::rgExpr, AST::rgExprValue, AST::rgIdentifier);
		reuse.loops.cursorname[2] = &cursorGet;
		auto& get = cursorGet.append(AST::rgExprSubDot, AST::rgIdentifier);
		get.text = "get";
		get.append(AST::rgCall);

		reuse.loops.scope = &scopeFor.append(AST::rgExpr, AST::rgExprValue, AST::rgScope);

		auto& cursorEnd = dowhile.append(AST::rgExpr, AST::rgExprValue, AST::rgIdentifier);
		reuse.loops.cursorname[3] = &cursorEnd;
		auto& next = cursorEnd.append(AST::rgExprSubDot, AST::rgIdentifier);
		next.text = "next";
		next.append(AST::rgCall);
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
