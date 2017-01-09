#include "scope.h"
#include <yuni/core/noncopyable.h>
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/ast/ast.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


namespace {


bool varValueInitialization(Scope& scope, uint32_t& localvar, AST::Node& varAssign,
							AST::Node& varnodeDecl, const AnyString& varname) {
	for (auto& assignChild : varAssign.children) {
		switch (assignChild.rule) {
			case AST::rgExpr: {
				bool success = scope.visitASTExpr(assignChild, localvar);
				if (unlikely(not success)) {
					error(varnodeDecl) << "could not declare '" << varname << "': error in initialization";
					return false;
				}
				break;
			}
			case AST::rgOperatorKind: {
				break; // operator =
			}
			default:
				return unexpectedNode(assignChild, "[var/init]");
		}
	}
	return true;
}


bool generateTypeofForClassVar(Scope& scope, uint32_t& lvid, AST::Node& varAssign) {
	// typeof (+2)
	//   tk-typeof, typeof
	//   call (+3)
	//       tk-parenthese-open, (
	//       call-parameter
	//       |   expr
	//       |     identifier: A
	auto typeofn = make_ref<AST::Node>(AST::rgTypeof);
	auto call = make_ref<AST::Node>(AST::rgCall);
	typeofn->children.push_back(call);
	// first parameter - the expr
	{
		auto param = make_ref<AST::Node>(AST::rgCallParameter);
		call->children.push_back(param);
		uint index = varAssign.findFirst(AST::rgExpr);
		if (unlikely(not (index < varAssign.children.size()))) {
			assert(false and "no node <expr> in <var-assign>");
			return false;
		}
		param->children.push_back(&(varAssign.children[index]));
	}
	return scope.visitASTExprTypeof(*typeofn, lvid);
}


bool generateInitFuncForClassVar(Scope& scope, const AnyString& varname, uint32_t lvid,
								 AST::Node& varAssign) {
	// name of the generated func for initialize the class variable
	ShortString64 funcName;
	funcName << "^default-var-%" << lvid << '-' << varname;
	assert(lvid > 0);
	// generating INIT
	//
	// variable member initialization is merely a call to the intrinsic `__nanyc_fieldset`
	// the first parameter is the result of the default value
	// the second parameter is the register where the name of member has been stored
	if (!scope.context.reuse.func.node)
		scope.context.prepareReuseForVariableMembers();
	scope.context.reuse.func.funcname->text = funcName;
	scope.context.reuse.func.varname->text  = varname;
	// Updating the EXPR
	uint index = varAssign.findFirst(AST::rgExpr);
	if (unlikely(not (index < varAssign.children.size()))) {
		assert(false and "no node <expr> in <var-assign>");
		return false;
	}
	scope.context.reuse.func.callparam->children.push_back(&(varAssign.children[index]));
	bool success = scope.visitASTFunc(*(scope.context.reuse.func.node));
	scope.context.reuse.func.callparam->children.clear();
	return success;
}


bool emitVarInClass(Scope& scope, const AnyString& varname, AST::Node& node, AST::Node* varType,
					AST::Node* varAssign, bool ref, bool constant) {
	auto& irout = scope.ircode();
	if (debugmode) {
		ir::emit::trace(irout);
		ir::emit::trace(irout, [&]() {
			return String{"class var "} << varname;
		});
	}
	// the new member variable
	auto mbvar = scope.nextvar();
	ir::emit::alloc(irout, mbvar);
	// the type of the expression
	uint32_t lvid = 0;
	if (varType != nullptr) {
		if (not scope.visitASTType(*varType, lvid))
			return false;
	}
	else {
		// type definition, via typeof(varAssign)
		if (not generateTypeofForClassVar(scope, lvid, *varAssign))
			return false;
	}
	if (unlikely(0 == lvid))
		return false;
	// follow
	{
		auto& operands    = irout.emit<isa::Op::follow>();
		operands.follower = mbvar;
		operands.lvid     = lvid;
		operands.symlink  = 0;
	}
	// preserve var / ref / const
	ir::emit::type::qualifierRef(irout, mbvar, ref);
	if (constant)
		ir::emit::type::qualifierConst(irout, mbvar, true);
	// variable definition
	scope.emitDebugpos(node);
	ir::emit::blueprint::var(irout, mbvar, varname);
	// generating an INIT func for the variable
	return generateInitFuncForClassVar(scope, varname, mbvar, *varAssign);
}


bool emitVarInFunc(Scope& scope, const AnyString& varname, AST::Node& node, AST::Node* varType,
		AST::Node* varAssign, bool ref, bool constant) {
	auto& irout = scope.ircode();
	// create the variable itself
	uint32_t varlvid = ir::emit::alloc(irout, scope.nextvar());
	if (varType != nullptr) {
		uint32_t lvidtype;
		if (not scope.visitASTType(*varType, lvidtype) or lvidtype == 0)
			return false;
		// follow
		auto& operands    = irout.emit<isa::Op::follow>();
		operands.follower = varlvid;
		operands.lvid     = lvidtype;
		operands.symlink  = 0;
	}
	// preserve var / ref / const
	ir::emit::type::qualifierRef(irout, varlvid, ref);
	if (constant)
		ir::emit::type::qualifierConst(irout, varlvid, true);
	// default value
	{
		uint32_t rhs = 0;
		ir::emit::ScopeLocker opscope{irout};
		if (not varValueInitialization(scope, rhs, *varAssign, node, varname))
			return false;
		assert(rhs != 0);
		if (not ref)
			ir::emit::assign(irout, varlvid, rhs, false);
		else {
			ir::emit::copy(irout, varlvid, rhs); // re-acquire to keep the value alive
			ir::emit::ref(irout, varlvid);
		}
	}
	// important: the alias must be declared *after* the right value
	// (otherwise it may be used by the code)
	scope.emitDebugpos(node); // reset the debug position
	ir::emit::namealias(irout, varlvid, varname);
	return true;
}


bool emitProperty(Scope& scope, const AnyString& varname, AST::Node& node, AST::Node* /*varType*/,
		AST::Node& varAssign, bool ref) {
	AST::Node* nodeGet = nullptr;
	AST::Node* nodeSet = nullptr;
	for (auto& object : varAssign.children) {
		switch (object.rule) {
			case AST::rgObject: {
				for (auto& child : object.children) {
					switch (child.rule) {
						case AST::rgObjectEntry: {
							bool validAST = ((child.children.size() == 2)
											 and child.children[0].rule == AST::rgIdentifier
											 and child.children[1].rule == AST::rgExpr);
							if (unlikely(not validAST))
								return (error(child) << "invalid AST node [property]");
							AnyString name = child.children[0].text;
							if (name == "get") {
								if (unlikely(nodeGet))
									return (error(child) << "property: multiple definition of 'get'");
								nodeGet = &(child.children[1]);
								break;
							}
							if (name == "set") {
								if (unlikely(nodeSet))
									return (error(child) << "property: multiple definition of 'set'");
								nodeSet = &(child.children[1]);
								break;
							}
							return (error(child) << "unsupported property component '" << name << "' (expected: 'get' or 'set')");
						}
						default:
							return unexpectedNode(child, "[property]");
					}
				}
				break;
			}
			case AST::rgExpr: {
				// read-only property
				if (unlikely(nodeGet))
					return (error(object) << "property: multiple definition of 'get'");
				nodeGet = &object;
				break;
			}
			default:
				return unexpectedNode(object, "[property/object]");
		}
	}
	if (unlikely(!nodeGet and !nodeSet))
		return (error(node) << "empty property definition");
	bool success = true;
	ShortString128 propname;
	if (nodeGet) {
		if (!scope.context.reuse.properties.get.node)
			scope.context.prepareReuseForPropertiesGET();
		propname << "^propget^" << varname;
		scope.context.reuse.properties.get.propname->text = propname;
		auto& type = *(scope.context.reuse.properties.get.type);
		type.children.clear();
		if (ref)
			type.children.push_back(scope.context.reuse.properties.get.typeIsRefAny);
		else
			type.children.push_back(scope.context.reuse.properties.get.typeIsAny);
		auto& returnValue = *(scope.context.reuse.properties.get.returnValue);
		returnValue.children.clear();
		returnValue.children.push_back(nodeGet);
		auto& funcnode = *(scope.context.reuse.properties.get.node);
		success &= scope.visitASTFunc(funcnode);
		// to avoid crap printed in the debugger
		scope.context.reuse.properties.get.propname->text.clear();
		type.children.clear();
		returnValue.children.clear();
	}
	if (nodeSet) {
		if (!scope.context.reuse.properties.set.node)
			scope.context.prepareReuseForPropertiesSET();
		propname.clear() << "^propset^" << varname;
		scope.context.reuse.properties.set.propname->text = propname;
		auto& body = *(scope.context.reuse.properties.set.body);
		body.children.clear();
		body.children.push_back(nodeSet);
		auto& funcnode = *(scope.context.reuse.properties.set.node);
		success &= scope.visitASTFunc(funcnode);
		// to avoid crap printed in the debugger
		scope.context.reuse.properties.set.propname->text.clear();
		body.children.clear();
	}
	return success;
}


} // namespace


bool Scope::visitASTVar(AST::Node& node) {
	assert(node.rule == AST::rgVar);
	assert(not node.children.empty());
	AnyString varname;
	bool ref = false;
	bool constant = false;
	AST::Node* varnodeDecl = nullptr;
	AST::Node* varAssign = nullptr;
	AST::Node* varType = nullptr;
	bool isProperty = false;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgIdentifier: {
				varnodeDecl = &child;
				varname = child.text;
				if (unlikely(not checkForValidIdentifierName(child, varname)))
					return false;
				break;
			}
			case AST::rgVarAssign: {
				varAssign = &child;
				break;
			}
			case AST::rgVarProperty: {
				isProperty = true;
				varAssign = &child;
				break;
			}
			case AST::rgVarType: {
				varType = &child;
				if (child.children.size() == 1 and child.children[0].rule == AST::rgType)
					varType = &(child.children[0]);
				else
					error(child) << "invalid type definition";
				break;
			}
			case AST::rgVarByValue: {
				break; // nothing to do
			}
			case AST::rgConst: {
				constant = true;
				warning(child) << "'const' in var declaration is currently not supported";
				break;
			}
			case AST::rgRef: {
				ref = true;
				break;
			}
			case AST::rgCref: {
				warning(child) << "'const' in var declaration is currently not supported";
				ref = true;
				constant = true;
				break;
			}
			case AST::rgFuncParamVariadic: {
				return (error(child) << "variadic parameter not allowed in variable definition");
			}
			default:
				return unexpectedNode(child, "[var]");
		}
	}
	if (unlikely(varnodeDecl == nullptr))
		return (ice() << "invalid null pointer for the var-decl node");
	if (unlikely(varAssign == nullptr)) // the variable currently must have a default value
		return (error(*varnodeDecl) << "value initialization is missing for '" << varname << '\'');
	if (not isProperty) {
		switch (kind) {
			case Kind::kfunc:
				return emitVarInFunc(*this, varname, *varnodeDecl, varType, varAssign, ref, constant);
			case Kind::kclass:
				return emitVarInClass(*this, varname, *varnodeDecl, varType, varAssign, ref, constant);
			default:
				return (ice(*varnodeDecl) << "var declaration: unsupported scope type");
		}
	}
	else {
		if (unlikely(constant))
			return (error(node) << "'const' keyword for properties is not supported yet");
		if (unlikely(varType))
			return (error(node) << "'type' for properties is not supported yet");
		switch (kind) {
			case Kind::kclass:
			case Kind::undefined:
				return emitProperty(*this, varname, *varnodeDecl, varType, *varAssign, ref);
			case Kind::kfunc:
				return (error(*varnodeDecl) << "properties in functions not implemented");
		}
	}
	return false;
}


} // namespace Producer
} // namespace ir
} // namespace ny
