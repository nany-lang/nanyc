#include "context.h"
#include "details/ast/ast.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


Context::Context(nybuild_cf_t& cf, AnyString filename, Sequence& ircode, Logs::Report report, bool ignoreAtoms)
	: ignoreAtoms(ignoreAtoms)
	, cf(cf)
	, ircode(ircode)
	, report(report)
	, dbgSourceFilename(filename)
	, localErrorHandler(this, &emitReportEntry)
	, localMetadataHandler(this, &retriveReportMetadata) {
}


Logs::Report Context::emitReportEntry(void* self, Logs::Level level) {
	auto& ctx = *(reinterpret_cast<Context*>(self));
	return ctx.report.fromErrLevel(level);
}


void Context::retriveReportMetadata(void* self, Logs::Level, const AST::Node* node, String& filename,
		uint32_t& line, uint32_t& offset) {
	auto& ctx = *(reinterpret_cast<Context*>(self));
	filename = ctx.dbgSourceFilename;
	if (node and node->offset > 0) {
		auto it = ctx.offsetToLine.lower_bound(node->offset);
		if (it != ctx.offsetToLine.end()) {
			if (it->first == node->offset or (--it != ctx.offsetToLine.end())) {
				line = it->second;
				offset = node->offset - it->first;
				return;
			}
		}
		line = 1;
		offset = 1;
	}
	else {
		line = 0;
		offset = 0;
	}
}


void Context::useNamespace(const AnyString& nmspc) {
	if (not nmspc.empty()) {
		nmspc.words(".", [&](const AnyString & part) -> bool {
			ir::emit::blueprint::namespacedef(ircode, part);
			return true;
		});
	}
}


void Context::generateLineIndexes(const AnyString& content) {
	uint32_t line = 1;
	const char* base = content.c_str();
	const char* end  = base + content.size();
	for (const char* c = base; c != end; ++c) {
		if (*c == '\n')
			offsetToLine.emplace(c - base, ++line);
	}
}


void Context::prepareReuseForStrings() {
	// new (+2)
	//     type-decl
	//     |   identifier: string
	auto& cache = reuse.string;
	cache.createObject = make_ref<AST::Node>(AST::rgNew);
	auto typeDecl = make_ref<AST::Node>(AST::rgTypeDecl);
	cache.createObject->children.push_back(typeDecl);
	auto classname = make_ref<AST::Node>(AST::rgIdentifier);
	typeDecl->children.push_back(classname);
	classname->text = "string";
}


void Context::prepareReuseForLiterals() {
	// new (+2)
	//   type-decl
	//   |   identifier: i64
	//   call (+3)
	//       call-parameter
	//           expr
	//               register: <lvid>
	auto& cache = reuse.literal;
	cache.node = make_ref<AST::Node>(AST::rgNew);
	auto& identifier = cache.node->append(AST::rgTypeDecl, AST::rgIdentifier);
	cache.classname = &identifier;
	auto& expr = cache.node->append(AST::rgCall, AST::rgCallParameter, AST::rgExpr);
	auto& reg  = expr.append(AST::rgRegister);
	cache.lvidnode = &reg;
}


void Context::prepareReuseForAsciis() {
	// new (+2)
	//   type-decl
	//   |   identifier: std
	//   |   type-sub-dot
	//   |       identifier Ascii
	//   call (+3)
	//       call-parameter
	//           expr
	//               register: <lvid>
	auto& cache = reuse.ascii;
	cache.node = make_ref<AST::Node>(AST::rgNew);
	auto& typedecl = cache.node->append(AST::rgTypeDecl);
	auto& identifier = typedecl.append(AST::rgIdentifier);
	identifier.text = "std";
	auto& classname = typedecl.append(AST::rgTypeSubDot, AST::rgIdentifier);
	classname.text = "Ascii";
	auto& expr = cache.node->append(AST::rgCall, AST::rgCallParameter, AST::rgExpr);
	auto& reg  = expr.append(AST::rgRegister);
	cache.lvidnode = &reg;
}


void Context::prepareReuseForClasses() {
	reuse.operatorDefault.node
		= AST::createNodeFunc(reuse.operatorDefault.funcname);
	reuse.operatorClone.node
		= AST::createNodeFuncCrefParam(reuse.operatorClone.funcname, "rhs");
}


void Context::prepareReuseForVariableMembers() {
	reuse.func.node = AST::createNodeFunc(reuse.func.funcname);
	auto funcBody = make_ref<AST::Node>(AST::rgFuncBody);
	(reuse.func.node)->children.push_back(funcBody);
	auto expr = make_ref<AST::Node>(AST::rgExpr);
	funcBody->children.push_back(expr);
	// intrinsic (+2)
	//     entity (+3)
	//     |   identifier: nanyc
	//     |   identifier: fieldset
	//     call (+7)
	//         call-parameter
	//         |   expr
	//         |       <expr A>
	//         call-parameter
	//         |   expr
	//         |       <expr B>
	auto intrinsic = make_ref<AST::Node>(AST::rgIntrinsic);
	expr->children.push_back(intrinsic);
	intrinsic->children.push_back(AST::createNodeIdentifier("^fieldset"));
	auto call = make_ref<AST::Node>(AST::rgCall);
	intrinsic->children.push_back(call);
	// param 2 - expr
	{
		reuse.func.callparam = make_ref<AST::Node>(AST::rgCallParameter);
		call->children.push_back(reuse.func.callparam);
	}
	// param text varname
	{
		auto callparam = make_ref<AST::Node>(AST::rgCallParameter);
		call->children.push_back(callparam);
		auto pexpr = make_ref<AST::Node>(AST::rgExpr);
		callparam->children.push_back(pexpr);
		reuse.func.varname = make_ref<AST::Node>(AST::rgStringLiteral);
		pexpr->children.push_back(reuse.func.varname);
	}
}


void Context::prepareReuseForClosures() {
	reuse.closure.node = make_ref<AST::Node>(AST::rgExpr);
	// expr
	//   expr-value
	//       new
	//           type-decl
	//               class
	//                   class-body
	//                       expr
	//                       |   expr-value
	//                       |     function (+2)
	//                       |         function-kind
	//                       |         |   function-kind-operator (+2)
	//                       |         |       function-kind-opname: ()
	//                       |         func-body
	//                       |             return-inline (+3)
	//                       |                 expr
	//                       |                 |   ...
	auto& exprValue = reuse.closure.node->append(AST::rgExprValue);
	auto& nnew      = exprValue.append(AST::rgNew);
	auto& typedecl  = nnew.append(AST::rgTypeDecl);
	auto& nclass    = typedecl.append(AST::rgClass);
	auto& cbody     = nclass.append(AST::rgClassBody);
	auto& bodyExpr  = cbody.append(AST::rgExpr);
	auto& bodyValue = bodyExpr.append(AST::rgExprValue);
	auto& func = bodyValue.append(AST::rgFunction);
	reuse.closure.classdef = &nclass;
	reuse.closure.func = &func;
	auto& funcname =
		func.append(AST::rgFunctionKind, AST::rgFunctionKindOperator, AST::rgFunctionKindOpname);
	funcname.text = "()";
	auto& params = func.append(AST::rgFuncParams);
	reuse.closure.params = &params;
	auto& rettype = func.append(AST::rgFuncReturnType);
	reuse.closure.rettype = &rettype;
	auto& funcBody = func.append(AST::rgFuncBody);
	reuse.closure.funcbody = &funcBody;
}


void Context::prepareReuseForIn() {
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
	//                                   |       type-decl
	//                                   |           identifier: bool
	//                                   func-body
	//                                       return
	//                                           expr
	//                                               expr-value
	//                                                   identifier: <predicate>
	//
	reuse.inset.node = make_ref<AST::Node>(AST::rgExpr);
	auto& exprValue = reuse.inset.node->append(AST::rgExprValue);
	auto& exprGroup = exprValue.append(AST::rgExprGroup);
	reuse.inset.container = &exprGroup;
	auto& viewname = exprValue.append(AST::rgExprSubDot, AST::rgIdentifier);
	reuse.inset.viewname = &viewname;
	auto& call = viewname.append(AST::rgCall);
	reuse.inset.call = &call;
	auto& paramExprValue = call.append(AST::rgCallParameter, AST::rgExpr, AST::rgExprValue);
	auto& func = paramExprValue.append(AST::rgFunction);
	func.append(AST::rgFunctionKind, AST::rgFunctionKindFunction);
	auto& funcparams = func.append(AST::rgFuncParams);
	auto& funcparam = funcparams.append(AST::rgFuncParam);
	funcparam.append(AST::rgCref);
	auto& cursorname = funcparam.append(AST::rgIdentifier);
	reuse.inset.elementname = &cursorname;
	auto& typend = func.append(AST::rgFuncReturnType, AST::rgType);
	auto& typedecl = typend.append(AST::rgTypeDecl, AST::rgIdentifier);
	typedecl.text = "bool";
	auto& funcbody = func.append(AST::rgFuncBody);
	auto& ret = funcbody.append(AST::rgReturn);
	reuse.inset.predicate = &ret;
	// -- always 'true' predicate
	// -- (when the predicate caluse `i in <set> | <predicate>` is missing)
	reuse.inset.premadeAlwaysTrue = make_ref<AST::Node>(AST::rgExpr);
	auto& trueNew = reuse.inset.premadeAlwaysTrue->append(AST::rgNew);
	auto& trueType = trueNew.append(AST::rgTypeDecl, AST::rgIdentifier);
	trueType.text = "bool";
	auto& trueExpr = trueNew.append(AST::rgCall, AST::rgCallParameter, AST::rgExpr);
	auto& trueIntrin = trueExpr.append(AST::rgIdentifier);
	trueIntrin.text = "__true";
}


void Context::prepareReuseForLoops() {
	// scope
	//     expr
	//         expr-value
	//            scope (+2)
	//                var (+3)
	//                |   ref
	//                |   identifier: cursor
	//                |   var-assign (+2)
	//                |       operator-kind
	//                |       |   operator-assignment: =
	//                |       expr
	//                |           expr-value
	//                |               identifier: myview
	//                |                   expr-sub-dot
	//                |                       identifier: cursor
	//                |                           call
	//                expr
	//                    expr-value
	//                        if (+2)
	//                            expr
	//                            |   expr-value
	//                            |       identifier: cursor
	//                            |           expr-sub-dot
	//                            |               identifier: findfirst
	//                            |                   call
	//                            if-then
	//                            |   expr
	//                            |       expr-value
	//                            |           scope
	//                            |               do-while (+2)
	//                            |                   expr
	//                            |                   |   expr-value
	//                            |                   |       scope (+2)
	//                            |                   |           var (+3)
	//                            |                   |           |   ref
	//                            |                   |           |   identifier: i
	//                            |                   |           |   var-assign (+2)
	//                            |                   |           |       operator-kind
	//                            |                   |           |       |   operator-assignment: =
	//                            |                   |           |       expr
	//                            |                   |           |           expr-value
	//                            |                   |           |               identifier: cursor
	//                            |                   |           |                   expr-sub-dot
	//                            |                   |           |                       identifier: get
	//                            |                   |           |                           call
	//                            |                   |           expr
	//                            |                   |               expr-value
	//                            |                   |                   scope
	//                            |                   |                       <.. user code here ..>
	//                            |                   expr
	//                            |                       expr-value
	//                            |                           identifier: cursor
	//                            |                               expr-sub-dot
	//                            |                                   identifier: next
	//                            |                                       call
	//                            else-then {optional}
	reuse.loops.node = make_ref<AST::Node>(AST::rgExpr);
	auto& scope = reuse.loops.node->append(AST::rgScope);
	// capture the cursor
	{
		auto& var = scope.append(AST::rgVar);
		var.append(AST::rgRef);
		reuse.loops.cursorname[0] = &var.append(AST::rgIdentifier);
		auto& varAssign = var.append(AST::rgVarAssign);
		auto& opassign = varAssign.append(AST::rgOperatorKind, AST::rgOperatorAssignment);
		opassign.text = "=";
		auto& value = varAssign.append(AST::rgExpr, AST::rgExprValue, AST::rgRegister);
		reuse.loops.viewlvid = &value;
		auto& cursorCall = value.append(AST::rgExprSubDot, AST::rgIdentifier);
		cursorCall.text = "cursor";
		cursorCall.append(AST::rgCall);
	}
	auto& ifFindFirst = scope.append(AST::rgExpr, AST::rgExprValue, AST::rgIf);
	reuse.loops.ifnode = &ifFindFirst;
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
	reuse.loops.elseClause = make_ref<AST::Node>(AST::rgIfElse);
	reuse.loops.elseScope = &(reuse.loops.elseClause->append(AST::rgExpr, AST::rgExprValue, AST::rgScope));
}


void Context::prepareReuseForPropertiesGET() {
	// function (+3)
	//     function-kind
	//     |   function-kind-function (+2)
	//     |       symbol-name
	//     |           identifier: ^prop^get^myprop
	//     func-return-type (+2)
	//     |   type
	//     |       type-qualifier
	//     |           ref
	//     |       type-decl
	//     |           identifier: any
	//     func-body
	//         scope (+4)
	//             expr
	//             |   expr-value
	//             |       return (+2)
	//             |           expr
	//             |               ...
	auto root = make_ref<AST::Node>(AST::rgFunction);
	reuse.properties.get.node = root;
	// function name
	auto& symbolname = root->append
					   (AST::rgFunctionKind, AST::rgFunctionKindFunction, AST::rgSymbolName);
	reuse.properties.get.propname = &symbolname.append(AST::rgIdentifier);
	// Return types
	reuse.properties.get.type = &root->append(AST::rgFuncReturnType, AST::rgType);
	reuse.properties.get.typeIsRefAny = make_ref<AST::Node>(AST::rgTypeQualifier);
	reuse.properties.get.typeIsRefAny->append(AST::rgRef);
	reuse.properties.get.typeIsAny = make_ref<AST::Node>(AST::rgTypeDecl);
	auto& any = reuse.properties.get.typeIsAny->append(AST::rgIdentifier);
	any.text = "any";
	// body
	auto& retnode = root->append(AST::rgFuncBody, AST::rgScope, AST::rgReturn);
	reuse.properties.get.returnValue = &retnode;
}


void Context::prepareReuseForPropertiesSET() {
	// function (+3)
	//     function-kind
	//     |   function-kind-function (+2)
	//     |       symbol-name
	//     |           identifier: piko
	//     func-params (+3)
	//     |   func-param (+2)
	//     |   |   cref
	//     |   |   identifier: value
	//     func-body
	//         scope
	auto root = make_ref<AST::Node>(AST::rgFunction);
	reuse.properties.set.node = root;
	auto& symbol = root->append(AST::rgFunctionKind, AST::rgFunctionKindFunction, AST::rgSymbolName);
	auto& identifier = symbol.append(AST::rgIdentifier);
	reuse.properties.set.propname = &identifier;
	// parameter "value"
	auto& param = root->append(AST::rgFuncParams, AST::rgFuncParam);
	param.append(AST::rgCref);
	auto& paramname = param.append(AST::rgIdentifier);
	paramname.text = "value";
	// body
	auto& body = root->append(AST::rgFuncBody, AST::rgScope);
	reuse.properties.set.body = &body;
}


void Context::prepareReuseForUnittest() {
	// function (+3)
	//     function-kind
	//     |   function-kind-function (+2)
	//     |       symbol-name
	//     |           identifier: ^unittest^<name>
	//     func-body
	//         .. scope ..
	auto root = make_ref<AST::Node>(AST::rgFunction);
	reuse.unittest.node = root;
	auto& symbol = root->append(AST::rgFunctionKind, AST::rgFunctionKindFunction, AST::rgSymbolName);
	auto& identifier = symbol.append(AST::rgIdentifier);
	reuse.unittest.funcname = &identifier;
	reuse.unittest.funcbody = & root->append(AST::rgFuncBody);
}


void Context::prepareReuseForAnonymObjects() {
	// expr-group
	// |   expr-value
	// |       new
	// |           type-decl
	// |               class
	// |                   class-body
	auto root = make_ref<AST::Node>(AST::rgExprGroup);
	reuse.object.node = root;
	auto& typedecl = root->append(AST::rgExprValue, AST::rgNew, AST::rgTypeDecl);
	reuse.object.classbody = &typedecl.append(AST::rgClass, AST::rgClassBody);
}


void Context::prepareReuseForShorthandArray() {
	// new (+2)
	//     type-decl (+2)
	//         identifier: std
	//         type-sub-dot (+3)
	//             identifier: Array
	//             expr-type-template (+3)
	//                 call-template-parameters
	//                 |   call-template-parameter
	//                 |       type
	//                 |           type-decl
	//                 |               typeof (+2)
	//                 |                   call (+5)
	//                 |                       call-parameter
	//                 |                       |   expr
	//                 |                       |       expr-value
	//                 |                       |           identifier: <value1>
	//                 |                       call-parameter
	//                 |                       |   expr
	//                 |                       |       expr-value
	//                 |                       |           identifier: <value2>
	auto newnode = make_ref<AST::Node>(AST::rgNew);
	reuse.shorthandArray.node = newnode;
	auto& typedecl = newnode->append(AST::rgTypeDecl);
	auto& stdname = typedecl.append(AST::rgIdentifier);
	stdname.text = "std";
	auto& subdot = typedecl.append(AST::rgTypeSubDot);
	auto& arrayname = subdot.append(AST::rgIdentifier);
	arrayname.text = "Array";
	auto& callparam = subdot.append(AST::rgExprTypeTemplate, AST::rgCallTemplateParameters,
									AST::rgCallTemplateParameter);
	auto& type = callparam.append(AST::rgType, AST::rgTypeDecl, AST::rgTypeof);
	reuse.shorthandArray.typeofcall = &type.append(AST::rgCall);
}


void Context::prepareReuseForScopeExit() {
	//  expr
	//      expr-value
	//          new
	//              type-decl
	//                  class
	//                      class-body
	//                          function (+2)
	//                              function-kind
	//                              |   function-kind-operator
	//                              |       function-kind-opname: dispose
	//                              func-body
	auto newnode = make_ref<AST::Node>(AST::rgExpr);
	reuse.scope.exit.node = newnode;
	auto& typeDecl = newnode->append(AST::rgExprValue, AST::rgNew, AST::rgTypeDecl);
	auto& func = typeDecl.append(AST::rgClass, AST::rgClassBody, AST::rgFunction);
	auto& dispose = func.append(AST::rgFunctionKind, AST::rgFunctionKindOperator, AST::rgFunctionKindOpname);
	dispose.text = "dispose";
	auto& body = func.append(AST::rgFuncBody);
	reuse.scope.exit.body = &body;
}


} // namespace Producer
} // namespace ir
} // namespace ny
