#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{

	namespace {


	struct FuncInspector final : public Yuni::NonCopyable<FuncInspector>
	{
		using FuncnameType = CString<Config::maxSymbolNameLength, false>;

		//! Default
		FuncInspector(Scope& scope);

		bool inspect(AST::Node& node);

		//! Parent scope
		Scope& scope;
		//! Func body
		AST::Node* body = nullptr;

		//! name of the function
		FuncnameType funcname;
		//! Flag to remember if the function is the new operator
		// (for 'self' parameters for example)
		bool isNewOperator = false;

	private:
		bool inspectVisibility(AST::Node&);
		bool inspectKind(AST::Node&);
		bool inspectParameters(AST::Node*, AST::Node*);
		bool inspectReturnType(AST::Node&);
		bool inspectSingleParameter(uint pindex, AST::Node&, uint32_t paramoffset);
		bool inspectAttributes(Attributes&);

		//! Flag to determine whether the 'self' parameter is implicit
		// (simply dictated if within a class)
		bool hasImplicitSelf = false;

	}; // class FuncInspector




	FuncInspector::FuncInspector(Scope& scope)
		: scope(scope)
		, hasImplicitSelf(scope.isWithinClass())
	{}


	bool FuncInspector::inspectVisibility(AST::Node& node)
	{
		bool success = true;

		if (false /* TODO within a class */)
		{
			// visibility per attribute is not allowed inside a class
			// (the visibility if set by region)
			error(node) << "visibility oer attribute is not allowed inside a class";
			success = false;
		}
		else
		{
			// visibility from the node
			nyvisibility_t visibility = nycstring_to_visibility_n(node.text.c_str(), node.text.size());
			if (likely(visibility != nyv_undefined))
			{
				// in the global namespace, only 'public' and 'internal' are accepted
				if (unlikely(visibility != nyv_public and visibility != nyv_internal))
				{
					visibility = nyv_internal;
					error(node) << "invalid visibility '" << node.text << "'";
					success = false;
				}
				// set the visibility
				scope.emitDebugpos(node);
				scope.sequence().emitVisibility(visibility);
			}
			else
			{
				// should really never happen...
				error(node) << "invalid visibility '" << node.text << "'";
				success = false;
			}
		}

		return success;
	}


	bool FuncInspector::inspectKind(AST::Node& node)
	{
		if (unlikely(node.children.size() != 1))
			return unexpectedNode(node, "[funckind/child]");

		auto& kindchild = node.children.front();
		switch (kindchild.rule)
		{
			//
			// func/operator definition
			//
			case AST::rgFunctionKindFunction:
			{
				if (unlikely(kindchild.children.size() != 1))
					return unexpectedNode(kindchild, "[funckindfunc/child]");

				auto& symbolname = kindchild.children.front();
				AnyString name = scope.getSymbolNameFromASTNode(symbolname);
				if (not checkForValidIdentifierName(symbolname, name))
					return false;
				funcname = name;
				break;
			}
			case AST::rgFunctionKindOperator:
			{
				if (unlikely(kindchild.children.size() != 1))
					return unexpectedNode(kindchild, "[funckindfunc/child]");

				auto& opname = kindchild.children.front();
				if (unlikely(opname.rule != AST::rgFunctionKindOpname or not opname.children.empty()))
					return unexpectedNode(opname, "[funckindfunc/child]");

				Flags<IdNameFlag> flags = IdNameFlag::isOperator;
				if (hasImplicitSelf) // not within a class
					flags += IdNameFlag::isInClass;
				if (not checkForValidIdentifierName(opname, opname.text, flags))
					return false;

				funcname = '^';
				if (opname.text != "dispose")
					funcname << opname.text;
				else
					funcname << "#user-dispose";

				isNewOperator = (opname.text == "new");
				break;
			}
			case AST::rgFunctionKindView:
			{
				if (kindchild.children.empty())
				{
					// default view
					funcname = "^view^default";
				}
				else
				{
					auto& symbolname = kindchild.children.front();
					AnyString name = scope.getSymbolNameFromASTNode(symbolname);
					if (not checkForValidIdentifierName(symbolname, name))
						return false;
					funcname.clear();
					funcname << "^view^" << name;
				}
				break;
			}
			default:
				return unexpectedNode(kindchild, "[funckind]");
		}
		return true;
	}


	bool FuncInspector::inspectSingleParameter(uint pindex, AST::Node& node, uint32_t paramoffset)
	{
		assert(node.rule == AST::rgFuncParam and "invalid func param node");

		// lvid for the parameter
		uint32_t lvid = pindex + 1 + 1; // params are 2-based (1 is the return type)
		// name of the parameter
		AnyString paramname;
		// exit status
		bool success = true;
		// operator new (self x)
		bool autoMemberAssignment = false;

		// we can have the following pattern: ref a: sometype
		// the qualifiers will be reset by the type definition (if any)
		// thus, if 'ref' or 'const' is provided, the opcodes must be generated _after_
		// ref parameter
		bool ref = false;
		// constant parameter
		bool constant = false;

		scope.emitDebugpos(node);

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgRef:   ref = true; break;
				case AST::rgConst: constant = true; break;
				case AST::rgCref:  ref = constant = true; break;

				case AST::rgIdentifier: // name of the parameter
				{
					if (checkForValidIdentifierName(child, child.text))
					{
						paramname = child.text;
					}
					else
					{
						paramname = scope.acquireString(String() << "_p_" << (void*)(&child));
						success = false;
					}
					break;
				}
				case AST::rgVarType:
				{
					LVID paramtype = 0;

					for (auto& childtype: child.children)
					{
						switch (childtype.rule)
						{
							case AST::rgType:
								{
									success &= scope.visitASTType(childtype, paramtype);
									break;
								}
							default:
								success &= unexpectedNode(childtype, "[func/param-type]");
						}
					}
					if (not lvidIsAny(paramtype) and paramtype != 0)
					{
						auto& operands    = scope.sequence().emit<ISA::Op::follow>();
						operands.follower = lvid;
						operands.lvid     = paramtype;
						operands.symlink  = 1;
					}
					break;
				}
				case AST::rgFuncParamSelf:
				{
					if (likely(isNewOperator))
					{
						autoMemberAssignment = true;
					}
					else
					{
						success = false;
						error(child)
							<< "automatic variable assignment is only allowed in the operator 'new'";
					}
					break;
				}
				case AST::rgFuncParamVariadic:
				{
					error(child) << "variadic parameters not implemented";
					success = false;
					break;
				}
				default:
					success = unexpectedNode(child, "[param]");
			}
		}

		if (unlikely(paramname.empty()))
		{
			// generating a pseudo identifier name {should never happen}
			ice(node) << "anonymous variable not allowed";
			success   = false;
			paramname = scope.acquireString(String() << "_p_" << (void*)(&node));
		}

		// update the parameter name within the IR code (whatever previous result)
		auto sid = scope.sequence().stringrefs.ref(paramname);
		// update the parameter opcode
		{
			auto& opparam = scope.sequence().at<ISA::Op::blueprint>(paramoffset);
			opparam.name  = sid;
			if (autoMemberAssignment)
				opparam.kind = (uint32_t) IR::ISA::Blueprint::paramself;
		}
		// the qualifiers may have been set by the type definition
		// thus they must be overriden and not always reset
		if (ref)
			scope.sequence().emitQualifierRef(lvid, true);
		if (constant)
			scope.sequence().emitQualifierConst(lvid, true);
		return success;
	}


	bool FuncInspector::inspectParameters(AST::Node* node, AST::Node* nodeTypeParams)
	{
		// total number of parameters
		uint32_t paramCount;
		if (node != nullptr)
		{
			assert(node->rule == AST::rgFuncParams and "invalid func params node");
			// determining the total number of parameters for the function
			// if within a class, a implicit parameter 'self' will be added to each function
			paramCount = node->children.size();
		}
		else
			paramCount = 0u;

		if (hasImplicitSelf) // implicit 'self' parameter
			++paramCount;

		// no parameter (not even 'self'), nothing to do here !
		if (paramCount == 0u)
		{
			// no parameters, but maybe some generic type parameters
			if (nodeTypeParams)
				return scope.visitASTDeclGenericTypeParameters(*nodeTypeParams);
			return true;
		}

		scope.emitDebugpos(node);
		bool success = true;

		if (unlikely(paramCount > Config::maxFuncDeclParameterCount - 1)) // too many parameters ?
		{
			assert(node != nullptr);
			error(*node) << "hard limit: too many parameters. Got "
				<< paramCount << ", expected: " << (Config::maxFuncDeclParameterCount - 1);
			success = false;
			paramCount = Config::maxFuncDeclParameterCount - 1;
		}

		// creating all blueprint parameters first to have their 'lvid' predictible
		// as a consequence, classdef for parameters start from 2 (0: null, 1: return type)
		// and variables for parameters start from 1
		auto& out = scope.sequence();

		// dealing first with the implicit parameter 'self'
		if (hasImplicitSelf)
		{
			uint32_t selfid = scope.nextvar();
			out.emitBlueprintParam(selfid, "self");
			out.emitSelf(selfid); // to resolve the type as 'self'
		}
		// iterating through all other user-defined parameters
		uint32_t offset = (hasImplicitSelf) ? 1u : 0u;
		if (paramCount - offset > 0U)
		{
			// already checked before
			assert(paramCount - offset < Config::maxPushedParameters);

			// declare all parameters first
			uint32_t paramOffsets[Config::maxPushedParameters];
			for (uint32_t i = offset; i < paramCount; ++i) // reserving lvid for each parameter
			{
				uint32_t opaddr = out.emitBlueprintParam(scope.nextvar(), nullptr);
				paramOffsets[i - offset] = opaddr;
			}

			// reserve registers (as many as parameters) for cloning parameters
			for (uint32_t i = 0u; i != paramCount; ++i)
				out.emitStackalloc(scope.nextvar(), nyt_any);

			// Generating IR for template parameters before the IR code for parameters
			// (especially for being able to use these types)
			if (nodeTypeParams)
				success &= scope.visitASTDeclGenericTypeParameters(*nodeTypeParams);
			// inspecting each parameter
			if (debugmode)
				out.emitComment("function parameters");
			// Generate IR (typing and default value) for each parameter
			assert(node != nullptr and "should not be here if there is no real parameter");
			for (uint32_t i = offset; i < paramCount; ++i)
				success &= inspectSingleParameter(i, node->children[i - offset], paramOffsets[i - offset]);
		}
		return success;
	}


	bool FuncInspector::inspectReturnType(AST::Node& node)
	{
		assert(node.rule == AST::rgFuncReturnType and "invalid return type node");
		assert(not node.children.empty() and " should been tested already");

		if (debugmode)
			scope.comment("return type"); // comment for clarity in code

		LVID rettype = 0;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgType:
				{
					bool ok = scope.visitASTType(child, rettype);
					if (unlikely(not ok))
						return false;
					break;
				}
				default:
					return unexpectedNode(child, "[func/ret-type]");
			}
		}

		if (not lvidIsAny(rettype))
		{
			scope.emitDebugpos(node);
			if (rettype == 0)
			{
				rettype = scope.createLocalBuiltinVoid(node);
				assert(rettype != 0);
			}

			assert(rettype != 0);
			auto& operands    = scope.sequence().emit<ISA::Op::follow>();
			operands.follower = 1; // params are 2-based (1 is the return type)
			operands.lvid     = rettype;
			operands.symlink  = 0;
		}

		if (debugmode)
			scope.comment("end return type"); // comment for clarity in code
		return true;
	}


	bool FuncInspector::inspectAttributes(Attributes& attrs)
	{
		auto& out = scope.sequence();

		if (attrs.flags(Attributes::Flag::shortcircuit))
		{
			out.emitPragmaShortcircuit(1);
			attrs.flags -= Attributes::Flag::shortcircuit;
		}
		if (attrs.flags(Attributes::Flag::builtinAlias))
		{
			assert(attrs.builtinAlias != nullptr);
			ShortString64 value;
			if (not AST::appendEntityAsString(value, *attrs.builtinAlias))
				return error(*attrs.builtinAlias) << "invalid builtinalias attribute";

			out.emitPragmaBuiltinAlias(value);
			attrs.flags -= Attributes::Flag::builtinAlias;
		}
		if (attrs.flags(Attributes::Flag::doNotSuggest))
		{
			out.emitPragmaSuggest(0);
			attrs.flags -= Attributes::Flag::doNotSuggest;
			assert(not attrs.flags(Attributes::Flag::doNotSuggest));
		}
		if (attrs.flags(Attributes::Flag::threadproc))
		{
			// currently silently ignored
			attrs.flags -= Attributes::Flag::threadproc;
		}
		return true;
	}


	bool FuncInspector::inspect(AST::Node& node)
	{
		// exit status
		bool success = true;
		// the node related to parameters
		AST::Node* nodeParams = nullptr;
		// the node related to template parameters
		AST::Node* nodeGenTParams = nullptr;
		// the node related to the return type
		AST::Node* nodeReturnType = nullptr;

		// function attributes, if any
		if (!!scope.attributes)
			success &= inspectAttributes(*scope.attributes);
		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgFunctionKind:   { success &= inspectKind(child); break; }
				case AST::rgFuncParams:     { nodeParams = &child; break; }
				case AST::rgVisibility:     { success &= inspectVisibility(child); break; }
				case AST::rgFuncReturnType: { if (not child.children.empty()) nodeReturnType = &child; break; }
				case AST::rgFuncBody:       { body = &child; break; }
				case AST::rgClassTemplateParams: { nodeGenTParams = &child; break; }
				default:
					success &= unexpectedNode(child, "[func]");
			}
		}

		if (funcname.empty())
			funcname = "<anonymous>";
		// the code currently has some prerequisites:
		// lvid %0: <invalid value>
		// lvid %1: the return type
		// lvid %2..N: the N parameters (if any)
		// lvid %2+N+1 .. %2+N+1+N: a reserved local variables for copying parameters
		//    (if required)
		// ...: generic type parameters if any
		// ...: definition of each parameters (and maybe variable cloning)

		// %1: the variable representing the return type (only a very simple definition)
		if (nodeReturnType)
			scope.createLocalBuiltinAny(node); // pre-create the return type
		else
			scope.createLocalBuiltinVoid(node);

		// ...then the parameters
		success &= inspectParameters(nodeParams, nodeGenTParams);
		// generate opcodes for return type verification (after parameters)
		if (nodeReturnType)
			success &= inspectReturnType(*nodeReturnType);
		return success;
	}


	} // anonymous namespace




	bool Scope::visitASTFunc(AST::Node& node)
	{
		assert(node.rule == AST::rgFunction);
		assert(not node.children.empty());

		if (unlikely(context.ignoreAtoms))
			return true;

		// new scope
		IR::Producer::Scope scope{*this};
		scope.moveAttributes(*this);
		// reset internal counter for generating local classdef in the current scope
		scope.resetLocalCounters();
		scope.kind = Scope::Kind::kfunc;
		scope.broadcastNextVarID = false;
		auto& out = sequence();

		// creating a new blueprint for the function
		uint32_t bpoffset = out.emitBlueprintFunc();
		uint32_t bpoffsiz = out.emitBlueprintSize();
		uint32_t bpoffsck = out.emitStackSizeIncrease();

		// making sure that debug info are available
		context.pPreviousDbgLine = (uint32_t) -1; // forcing debug infos
		scope.addDebugCurrentFilename();
		scope.emitDebugpos(node);

		bool success = true;
		bool isOperator = false;

		// evaluate the whole function, and grab the node body for continuing evaluation
		auto* body = ([&]() -> AST::Node*
		{
			FuncInspector inspector{scope};
			success = inspector.inspect(node);

			isOperator = (inspector.funcname.first() == '^');
			// update the func name
			auto sid = out.stringrefs.ref(inspector.funcname);
			out.at<ISA::Op::blueprint>(bpoffset).name = sid;
			return inspector.body;
		})();

		// body start (for auto generation)
		out.emitPragmaFuncBody();

		if (likely(body != nullptr))
		{
			if (debugmode)
				scope.comment("\nfunc body"); // comment for clarity in code
			// continue evaluating the func body independantly of the previous data and results
			for (auto& stmtnode: body->children)
				success &= scope.visitASTStmt(stmtnode);
		}

		// end of the blueprint
		out.emitEnd();
		uint32_t blpsize = out.opcodeCount() - bpoffset;
		out.at<ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		out.at<ISA::Op::stacksize>(bpoffsck).add = scope.nextVarID + 1u;
		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
