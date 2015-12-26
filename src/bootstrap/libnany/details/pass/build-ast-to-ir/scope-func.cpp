#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/grammar/nany.h"
#include "libnany-config.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{

	namespace // anonymous
	{

		class FuncInspector final : public Yuni::NonCopyable<FuncInspector>
		{
		public:
			typedef CString<Config::maxSymbolNameLength, false> FuncnameType;


		public:
			//! Default
			FuncInspector(Scope& scope);

			bool inspect(Node& node);


		public:
			//! Parent scope
			Scope& scope;
			//! Func body
			Node* body = nullptr;

			//! name of the function
			FuncnameType funcname;
			//! Flag to remember if the function is the new operator
			// (for 'self' parameters for example)
			bool isNewOperator = false;
			// number of parameters of the function
			uint paramCount = 0;

			// IR debuginfo
			bool hasIRDebuginfo = false;

		private:
			bool inspectVisibility(Node&);
			bool inspectKind(Node&);
			bool inspectParameters(Node*);
			bool inspectReturnType(Node&);
			bool inspectSingleParameter(uint pindex, Node&, uint32_t paramoffset);

		private:
			bool pWithinClass = false;

		}; // class FuncInspector







		inline FuncInspector::FuncInspector(Scope& scope)
			: scope(scope)
			, pWithinClass(scope.isWithinClass())
		{}


		inline bool FuncInspector::inspectVisibility(Node& node)
		{
			bool success = true;

			if (false /* TODO within a class */)
			{
				// visibility per attribute is not allowed inside a class
				// (the visibility if set by region)
				scope.error(node) << "visibility oer attribute is not allowed inside a class";
				success = false;
			}
			else
			{
				// visibility from the node
				nyvisibility_t visibility = nany_cstring_to_visibility_n(node.text.c_str(), node.text.size());

				if (likely(visibility != nyv_undefined))
				{
					// in the global namespace, only 'public' and 'internal' are accepted
					if (unlikely(visibility != nyv_public and visibility != nyv_internal))
					{
						visibility = nyv_internal;
						scope.error(node) << "invalid visibility '" << node.text << "'";
						success = false;
					}

					// set the visibility
					scope.program().emitVisibility(visibility);
				}
				else
				{
					// should really never happen...
					scope.error(node) << "invalid visibility '" << node.text << "'";
					success = false;
				}
			}

			return success;
		}


		inline bool FuncInspector::inspectKind(Node& node)
		{
			if (unlikely(node.children.size() != 1))
				return scope.ICEUnexpectedNode(node, "[funckind/child]");

			auto& kindchild = *(node.children[0]);
			switch (kindchild.rule)
			{
				//
				// func/operator definition
				//
				case rgFunctionKindFunction:
				{
					if (unlikely(kindchild.children.size() != 1))
						return scope.ICEUnexpectedNode(kindchild, "[funckindfunc/child]");

					auto& symbolname = *(kindchild.children[0]);
					AnyString name = scope.getSymbolNameFromASTNode(symbolname);
					if (not checkForValidIdentifierName(scope.report(), symbolname, name, false))
						return false;
					funcname = name;
					break;
				}

				case rgFunctionKindOperator:
				{
					if (unlikely(kindchild.children.size() != 1))
						return scope.ICEUnexpectedNode(kindchild, "[funckindfunc/child]");

					auto& opname = *(kindchild.children[0]);
					if (unlikely(opname.rule != rgFunctionKindOpname or not opname.children.empty()))
						return scope.ICEUnexpectedNode(opname, "[funckindfunc/child]");

					if (not checkForValidIdentifierName(scope.report(), opname, opname.text, true))
						return false;

					funcname.clear();
					funcname << '^' << opname.text;

					isNewOperator = (opname.text == "new");
					break;
				}

				default:
					return scope.ICEUnexpectedNode(kindchild, "[funckind]");
			}
			return true;
		}


		inline bool FuncInspector::inspectSingleParameter(uint pindex, Node& node, uint32_t paramoffset)
		{
			assert(node.rule == rgFuncParam and "invalid func param node");

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


			for (auto& childptr: node.children)
			{
				auto& child = *childptr;

				switch (child.rule)
				{
					case rgRef:   ref = true; break;
					case rgConst: constant = true; break;
					case rgCref:  ref = constant = true; break;

					case rgIdentifier: // name of the parameter
					{
						if (checkForValidIdentifierName(scope.report(), child, child.text, false))
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

					case rgVarType:
					{
						LVID paramtype = 0;

						for (auto& childtypeptr: child.children)
						{
							auto& childtype = *childtypeptr;
							switch (childtype.rule)
							{
								case rgType:
								{
									success &= scope.visitASTType(childtype, paramtype);
									break;
								}
								default:
									success &= scope.ICEUnexpectedNode(childtype, "[func/param-type]");
							}
						}

						if (not lvidIsAny(paramtype) and paramtype != 0)
						{
							auto& operands    = scope.program().emit<ISA::Op::follow>();
							operands.follower = lvid;
							operands.lvid     = paramtype;
							operands.symlink  = 1;
						}
						break;
					}

					case rgFuncParamSelf:
					{
						if (likely(isNewOperator))
						{
							autoMemberAssignment = true;
						}
						else
						{
							success = false;
							scope.error(child)
								<< "automatic variable assignment is only allowed in the operator 'new'";
						}
						break;
					}

					default:
						success = scope.ICEUnexpectedNode(child, "[param]");
				}
			}

			if (unlikely(paramname.empty()))
			{
				// generating a pseudo identifier name {should never happen}
				scope.ICE(node) << "anonymous variable not allowed";
				success   = false;
				paramname = scope.acquireString(String() << "_p_" << (void*)(&node));
			}

			// update the parameter name within the IR code (whatever previous result)
			auto sid = scope.program().stringrefs.ref(paramname);

			// update the parameter opcode
			{
				auto& opparam = scope.program().at<ISA::Op::pragma>(paramoffset);
				opparam.value.param.name = sid;
				if (autoMemberAssignment)
					opparam.pragma = static_cast<uint32_t>(ISA::Pragma::blueprintparamself);
			}

			// the qualifiers may have been set by the type definition
			// thus they must be overriden and not always reset
			if (ref)
				scope.program().emitQualifierRef(lvid, true);
			if (constant)
				scope.program().emitQualifierConst(lvid, true);
			return success;
		}


		bool FuncInspector::inspectParameters(Node* node)
		{
			if (node != nullptr)
			{
				assert(node->rule == rgFuncParams and "invalid func params node");
				// determining the total number of parameters for the function
				// if within a class, a implicit parameter 'self' will be added to each function
				paramCount = (uint) node->children.size();
			}
			else
				paramCount = 0;

			if (pWithinClass)
				++paramCount; // self

			// no parameter (not even 'self'), nothing to do here !
			if (paramCount == 0)
				return true;


			// some comments for debugging
			if (unlikely(hasIRDebuginfo))
				scope.comment("raw input parameters");

			bool success = true;

			if (unlikely(paramCount > Config::maxFuncDeclParameterCount - 1)) // too many parameters ?
			{
				assert(node != nullptr);
				scope.error(*node) << "too many parameters. Got " << paramCount << ", expected: "
					<< (Config::maxFuncDeclParameterCount - 1);
				success = false;
				paramCount = Config::maxFuncDeclParameterCount - 1;
			}

			// creating all blueprint parameters first to have their 'lvid' predictible
			// as a consequence, classdef for parameters start from 2 (0: null, 1: return type)
			// and variables for parameters start from 1
			auto& out = scope.program();

			// dealing first with the implicit parameter 'self'
			if (pWithinClass)
			{
				uint32_t selfid = scope.nextvar();
				out.emitBlueprintParam(selfid, "self");
				out.emitSelf(selfid); // must be resolved as 'self'
			}

			// iterating through all other user-defined parameters
			uint offset = (pWithinClass) ? 1 : 0;
			if (paramCount - offset > 0)
			{
				// already checked before
				assert(paramCount - offset < Config::maxPushedParameters);

				// adding all parameters first
				uint32_t paramOffsets[Config::maxPushedParameters];
				for (uint i = offset; i < paramCount; ++i) // reserving lvid for each parameter
				{
					auto opaddr = out.emitBlueprintParam(scope.nextvar(), nullptr);
					paramOffsets[i - offset] = opaddr;
				}

				// reserve registers as many as parameters for cloning parameters
				out.emitComment("START !!");
				for (uint32_t i = 0; i != paramCount; ++i)
					out.emitStore(scope.nextvar(), nyt_any);

				// inspecting each parameter
				out.emitComment("parameters definition");

				assert(node != nullptr and "should not be here if there is no real parameter");
				for (uint i = offset; i < paramCount; ++i)
					success &= inspectSingleParameter(i, *(node->children[i - offset]), paramOffsets[i - offset]);
			}
			else
			{
				// produce reserved registers for consistency, even if not used
				for (uint32_t i = 0; i != paramCount; ++i)
					out.emitStore(scope.nextvar(), nyt_any);
			}

			return success;
		}


		inline bool FuncInspector::inspectReturnType(Node& node)
		{
			assert(node.rule == rgFuncReturnType and "invalid return type node");
			if (unlikely(hasIRDebuginfo))
				scope.comment("return type"); // comment for clarity in code

			bool success = true;
			LVID rettype = 0;

			for (auto& childptr: node.children)
			{
				auto& child = *childptr;
				switch (child.rule)
				{
					case rgType:
					{
						success &= scope.visitASTType(child, rettype);
						break;
					}
					default:
						success &= scope.ICEUnexpectedNode(child, "[func/ret-type]");
				}
			}

			if (not lvidIsAny(rettype))
			{
				if (rettype != 0)
				{
					auto& operands    = scope.program().emit<ISA::Op::follow>();
					operands.follower = 1; // params are 2-based (1 is the return type)
					operands.lvid     = rettype;
					operands.symlink  = 1;
				}
				else
					scope.createLocalBuiltinVoid(node);
			}
			return success;
		}


		inline bool FuncInspector::inspect(Node& node)
		{
			// exit status
			bool success = true;
			// the node related to parameters
			Node* nodeParams = nullptr;
			// the node related to the return type
			Node* nodeReturnType = nullptr;

			for (auto& childptr: node.children)
			{
				auto& child = *childptr;
				switch (child.rule)
				{
					case rgFunctionKind:   { success &= inspectKind(child); break; }
					case rgFuncParams:     { nodeParams = &child; break; }
					case rgVisibility:     { success &= inspectVisibility(child); break; }
					case rgFuncReturnType: { nodeReturnType = &child; break; }
					case rgFuncBody:       { body = &child; break; }
					default:
						success &= scope.ICEUnexpectedNode(child, "[func]");
				}
			}

			// the variable representing the return type (only a very simple definition)
			if (nodeReturnType)
				scope.createLocalBuiltinAny(node); // pre-create the return type
			else
				scope.createLocalBuiltinVoid(node);

			// ...then the parameters
			success &= inspectParameters(nodeParams);

			// generate opcodes for return type verification (after parameters)
			if (nodeReturnType)
				success &= inspectReturnType(*nodeReturnType);

			return success;
		}



	} // anonymous namespace









	bool Scope::visitASTFunc(Node& node)
	{
		assert(node.rule == rgFunction);
		assert(not node.children.empty());

		// new scope
		IR::Producer::Scope scope{*this};
		// reset internal counter for generating local classdef in the current scope
		scope.resetLocalCounters();
		scope.kind = Scope::Kind::kfunc;
		scope.broadcastNextVarID = false;

		// creating a new blueprint for the function
		uint32_t bpoffset = program().emitBlueprintFunc();
		uint32_t bpoffsiz = program().emitBlueprintSize();
		uint32_t bpoffsck = program().emitStackSizeIncrease();

		// making sure that debug info are available
		scope.addDebugCurrentFilename();
		scope.emitDebugpos(node);


		bool success = true;
		bool hasIRDebuginfo = scope.hasIRDebuginfo();
		bool isOperator = false;

		// evaluate the whole function, and grab the node body for continuing evaluation
		Node* body = ([&]() -> Node*
		{
			FuncInspector inspector{scope};
			inspector.hasIRDebuginfo = hasIRDebuginfo;
			success = inspector.inspect(node);

			isOperator = (inspector.funcname.first() == '^');
			// update the func name
			auto sid = program().stringrefs.ref(inspector.funcname);
			program().at<ISA::Op::pragma>(bpoffset).value.blueprint.name = sid;
			return inspector.body;
		})();

		// body start (for auto generation)
		program().emitPragmaFuncBody();

		if (likely(body != nullptr))
		{
			if (unlikely(hasIRDebuginfo))
				scope.comment("\nfunc body"); // comment for clarity in code

			// continue evaluating the func body independantly of the previous data and results
			for (auto& stmtnode: body->children)
				success &= scope.visitASTStmt(*stmtnode);
		}

		// end of the blueprint
		program().emitEnd();
		uint32_t blpsize = program().opcodeCount() - bpoffset;
		program().at<ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		program().at<ISA::Op::stacksize>(bpoffsck).add = scope.pNextVarID + 1;
		return success;
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
