#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/ast/ast.h"
#include "libnany-config.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	inline bool Scope::visitASTVarValueInitialization(LVID& localvar, const AST::Node& varAssign,
		const AST::Node& varnodeDecl, const AnyString& varname)
	{
		for (auto& assignptr: varAssign.children)
		{
			auto& assignChild = *assignptr;
			switch (assignChild.rule)
			{
				case AST::rgExpr:
				{
					setErrorFrom(assignChild);
					bool success = visitASTExpr(assignChild, localvar);

					if (unlikely(not success))
					{
						error(varnodeDecl) << "could not declare '" << varname << "': error in initialization";
						return false;
					}
					break;
				}
				case AST::rgOperatorKind:
				{
					break; // operator =
				}
				default:
					return ICEUnexpectedNode(assignChild, "[var]");
			}
		}
		return true;
	}


	inline bool Scope::generateTypeofForClassVar(LVID& lvid, const AST::Node& varAssign)
	{
		// typeof (+2)
		//	 tk-typeof, typeof
		//	 call (+3)
		//		 tk-parenthese-open, (
		//		 call-parameter
		//		 |   expr
		//		 |	   identifier: A
		AST::Node::Ptr typeofn = new AST::Node{AST::rgTypeof};

		AST::Node::Ptr call = new AST::Node{AST::rgCall};
		typeofn->children.push_back(call);

		// first parameter - the expr
		{
			AST::Node::Ptr param = new AST::Node{AST::rgCallParameter};
			call->children.push_back(param);

			uint index = varAssign.findFirst(AST::rgExpr);
			if (unlikely(not (index < varAssign.children.size())))
			{
				assert(false and "no node <expr> in <var-assign>");
				return false;
			}
			param->children.push_back(varAssign.children[index]);
		}
		return visitASTExprTypeof(*typeofn, lvid);
	}


	inline bool
	Scope::generateInitFuncForClassVar(const AnyString& varname, LVID lvid, const AST::Node& varAssign)
	{
		// name of the generated func for initialize the class variable
		ShortString64 funcName;
		funcName << "^default-var-%" << lvid << '-' << varname;
		assert(lvid > 0);


		// generating INIT
		//
		// variable member initialization is merely a call to the intrinsic `__nanyc_fieldset`
		// the first parameter is the result of the default value
		// the second parameter is the register where the name of member has been stored

		if (!context.reuse.func.node)
			context.prepareReuseForVariableMembers();

		context.reuse.func.funcname->text = funcName;
		context.reuse.func.varname->text  = varname;

		// Updating the EXPR
		uint index = varAssign.findFirst(AST::rgExpr);
		if (unlikely(not (index < varAssign.children.size())))
		{
			assert(false and "no node <expr> in <var-assign>");
			return false;
		}
		context.reuse.func.callparam->children.push_back(varAssign.children[index]);

		bool success = visitASTFunc(*(context.reuse.func.node));

		context.reuse.func.callparam->children.clear();
		return success;
	}



	bool Scope::visitASTVar(const AST::Node& node)
	{
		assert(node.rule == AST::rgVar);
		assert(not node.children.empty());

		// variable name
		AnyString varname;
		bool ref = false;
		bool constant = false;
		AST::Node* varnodeDecl = nullptr;
		AST::Node* varAssign = nullptr;
		AST::Node* varType = nullptr;
		bool isProperty = false;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgIdentifier:
				{
					varnodeDecl = &child;
					varname = child.text;
					setErrorFrom(child);
					if (unlikely(not checkForValidIdentifierName(context.report, child, varname, false)))
						return false;
					break;
				}
				case AST::rgVarAssign:
				{
					varAssign = &child;
					break;
				}
				case AST::rgVarProperty:
				{
					isProperty = true;
					varAssign = &child;
					break;
				}

				case AST::rgVarType:
				{
					varType = &child;
					if (child.children.size() == 1 and child.children[0]->rule == AST::rgType)
						varType = AST::Node::Ptr::WeakPointer(child.children[0]);
					else
						error(child) << "invalid type definition";
					break;
				}

				case AST::rgVarByValue:
				{
					break; // nothing to do
				}
				case AST::rgConst:
				{
					constant = true;
					warning(child) << "'const' in var declaration is currently not supported";
					break;
				}
				case AST::rgRef:
				{
					ref = true;
					break;
				}
				case AST::rgCref:
				{
					warning(child) << "'const' in var declaration is currently not supported";
					ref = true;
					constant = true;
					break;
				}

				case AST::rgFuncParamVariadic:
				{
					error(child) << "variadic parameter not allowed in variable definition";
					return false;
				}

				default:
					return ICEUnexpectedNode(child, "[var]");
			}
		}

		if (unlikely(varnodeDecl == nullptr))
		{
			report().ICE() << "invalid null pointer for the var-decl node";
			return false;
		}

		if (unlikely(varAssign == nullptr)) // the variable currently must have a default value
		{
			error(*varnodeDecl) << "value initialization is missing for '" << varname << "'";
			return false;
		}

		auto& out = sequence();

		if (not isProperty)
		{
			switch (kind)
			{
				case Kind::kclass:
				{
					if (debugmode)
					{
						out.emitComment();
						out.emitComment(String{"class var "} << varname);
					}
					// the new member variable
					auto mbvar = nextvar();
					out.emitStackalloc(mbvar, nyt_any);

					// the type of the expression
					LVID lvid = 0;

					if (varType != nullptr)
					{
						if (not visitASTType(*varType, lvid))
							return false;
					}
					else
					{
						// type definition, via typeof(varAssign)
						if (not generateTypeofForClassVar(lvid, *varAssign))
							return false;
					}
					if (unlikely(0 == lvid))
						return false;

					// follow
					{
						auto& operands    = out.emit<ISA::Op::follow>();
						operands.follower = mbvar;
						operands.lvid     = lvid;
						operands.symlink  = 0;
					}
					// preserve var / ref / const
					out.emitQualifierRef(mbvar, ref);
					if (constant)
						out.emitQualifierConst(mbvar, true);

					// variable definition
					emitDebugpos(*varnodeDecl);
					out.emitBlueprintVardef(mbvar, varname);

					// generating an INIT func for the variable
					if (not generateInitFuncForClassVar(varname, mbvar, *varAssign))
						return false;
					break;
				}

				case Kind::kfunc:
				{
					// create the variable itself
					uint32_t varlvid  = out.emitStackalloc(nextvar(), nyt_any);

					if (varType != nullptr)
					{
						uint32_t lvidtype;
						if (not visitASTType(*varType, lvidtype) or lvidtype == 0)
							return false;

						// follow
						auto& operands    = out.emit<ISA::Op::follow>();
						operands.follower = varlvid;
						operands.lvid	  = lvidtype;
						operands.symlink  = 0;
					}

					// preserve var / ref / const
					out.emitQualifierRef(varlvid, ref);
					if (constant)
						out.emitQualifierConst(varlvid, true);

					{
						LVID rhs = 0;
						IR::OpcodeScopeLocker opscope{out};
						if (not visitASTVarValueInitialization(rhs, *varAssign, *varnodeDecl, varname))
							return false;

						assert(rhs != 0);
						if (not ref)
						{
							out.emitAssign(varlvid, rhs, false);
						}
						else
						{
							out.emitStore(varlvid, rhs); // re-acquire to keep the value alive
							out.emitRef(varlvid);
						}
					}

					// important: the alias must be declared *after* the right value
					// (otherwise it may be used by the code)
					emitDebugpos(*varnodeDecl); // reset the debug position
					out.emitNameAlias(varlvid, varname);
					break;
				}
				default:
				{
					ICE(*varnodeDecl) << "var declaration: invalid scope type";
					return false;
				}
			}
		}
		else
		{
			// the declaration is a property
			error(*varnodeDecl) << "properties not implemented";
			return false;
		}

		return true;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
