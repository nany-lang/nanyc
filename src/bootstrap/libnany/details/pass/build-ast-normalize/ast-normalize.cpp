#include "details/context/source.h"
#include "details/sema/metadata.h"
#include "details/ast/ast.h"
#include "details/context/build-info.h"
#include "details/reporting/report.h"
#include "libnany-config.h"
#include "libnany-traces.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include <functional>

using namespace Yuni;





namespace Nany
{


	namespace // anonymous
	{

		class ASTReplicator final
		{
		public:
			explicit ASTReplicator(ASTHelper& ast, Logs::Report);

			bool run(Node& fileRootnode, Node& newroot);


		public:
			//! File namespace (alias)
			std::pair<YString, Nany::Node*> nmspc;


		private:
			bool duplicateNode(Nany::Node& out, const Nany::Node& node);
			void iterateThroughChildren(const Nany::Node& node, Nany::Node& newNode);
			void collectNamespace(const Nany::Node& node);

			bool generateErrorFromErrorNode(const Nany::Node& node);

			void normalizeExpression(Nany::Node& node);
			void normalizeExprReorderOperators(Nany::Node& node);
			void normalizeExprTransformOperatorsToFuncCall(Nany::Node& node);
			void transformExprNodeToFuncCall(Nany::Node& node);
			void transformExprAssignmentToFuncCall(Nany::Node& node);

			void appendNewBoolNode(Node& parent, bool onoff);


		private:
			struct Frame final
			{
				nyvisibility_t visibility = nyv_default;
			};


		private:
			//! AST helper
			ASTHelper& ast;
			//! Logs
			Logs::Report report;

			YString errmsg;
			bool pDuplicationSuccess = true;

		}; // class ASTReplicator




		inline ASTReplicator::ASTReplicator(ASTHelper& ast, Logs::Report report)
			: ast(ast)
			, report(report)
		{
		}


		inline void ASTReplicator::iterateThroughChildren(const Nany::Node& node, Nany::Node& newNode)
		{
			node.each([&] (const Nany::Node& subnode) -> bool
			{
				duplicateNode(newNode, subnode);
				return true;
			});
		}


		void ASTReplicator::collectNamespace(const Nany::Node& node)
		{
			auto entity = node.xpath({rgEntity});
			assert(!(!entity) and "mismatch grammar");
			nmspc.first.clear();
			nmspc.second = const_cast<Nany::Node*>(&node);
			entity->extractChildText(nmspc.first, Nany::rgIdentifier, ".");

			uint32_t depth = nmspc.first.countChar('.');
			if (depth + 1 >= Config::maxNamespaceDepth)
			{
				report.error() << "too many namespaces";
				pDuplicationSuccess = false;
				nmspc.first = "__error__";
			}
		}


		inline void ASTReplicator::transformExprNodeToFuncCall(Nany::Node& node)
		{
			// AST structure: foo() < expr;  (raw output from the parser)
			//
			// - identifier: a
			//      - expr-comparison
			//      - operator-comparison: <
			//            - identifier: b
			//
			// after the first transformation (what we got in the input of the current method):
			//
			// - expr-comparison
			//     - identifier: a
			//     - operator-comparison: <
			//     - identifier: b
			//
			assert(node.children.size() == 3);
			uint32_t lhsIndex = 0;
			uint32_t opIndex  = 1;
			uint32_t rhsIndex = 2;

			// operator name normalization
			// nany uses a special character '^' to distinguish functions from operators
			// (since operators may have a real name)
			// reminder: strings are not owned by the AST
			AnyString opname = normalizeOperatorName(node.children[opIndex]->text);

			// not an assignment, the input will be transformed into a global func call:
			//
			//  - identifier: <, >, !=, ...
			//      - call
			//           - call-parameter
			//                - a
			//           - call-parameter
			//                - b
			//

			// promote the current node before any changes within the subtree
			ast.nodeRulePromote(node, rgIdentifier);
			node.text = opname;

			// create a new subtree
			auto& call = *ast.nodeCreate(rgCall);
			auto& lhs  = *ast.nodeAppend(call, {rgCallParameter, rgExpr});
			auto& rhs  = *ast.nodeAppend(call, {rgCallParameter, rgExpr});

			// re-parent rhs first, otherwise the index will be invalidated
			ast.nodeReparentAtTheEnd(*(node.children[rhsIndex]), node, rhsIndex, rhs);
			// re-parent lhs
			ast.nodeReparentAtTheEnd(*(node.children[lhsIndex]), node, lhsIndex, lhs);

			// remove all remaining nodes
			node.children.clear();

			// re-parent the new node 'call'
			AST::metadata(call).parent = &node;
			node.children.push_back(&call);
		}


		inline void ASTReplicator::transformExprAssignmentToFuncCall(Nany::Node& node)
		{
			// AST structure: foo() += expr; - or anything that should be asked to the object itself
			//
			//  - lhs identifier (arbitrary example)
			//      - call
			//  - expr-assignment ................ [&node]
			//      - operator-assignment: +=, -=...
			//      - rhs
			//
			assert(node.children.size() == 2);
			auto& operatorNode = *(node.children[0]);
			uint32_t rhsIndex = 1;
			auto& rhs = *(node.children[rhsIndex]);
			assert(operatorNode.rule == rgOperatorAssignment);

			// the operator is not an assignment. the input will be transformed into
			//
			//  - lhs identifier (arbitrary example)
			//      - call
			//  - expr-sub-dot ....................[&node]
			//      - identifier: +=, -=...
			//          - call
			//              - parameter
			//                   - rhs
			//
			ast.nodeRulePromote(node, rgExprSubDot);
			ast.nodeRulePromote(operatorNode, rgIdentifier);

			// normalizing the operator
			operatorNode.text = normalizeOperatorName(operatorNode.text);

			auto& call = *ast.nodeAppend(operatorNode, rgCall);
			auto& expr = *ast.nodeAppend(call, {rgCallParameter, rgExpr});

			ast.nodeReparentAtTheEnd(rhs, node, rhsIndex, expr);
		}



		void ASTReplicator::normalizeExprTransformOperatorsToFuncCall(Nany::Node& node)
		{
			// go for children, the container may change between each iteration
			for (uint i = 0; i < (uint) node.children.size(); )
			{
				Nany::Node& child = *(node.children[i]);

				switch (child.rule)
				{
					// transform all expressions 'lhs <operator> rhs' into a global func call
					// (like in 'a < b' -> '<(a, b)')
					case Nany::rgExprAdd:
					case Nany::rgExprStream:
					case Nany::rgExprComparison:
					case Nany::rgExprLogic:
					case Nany::rgExprLogicAnd:
					case Nany::rgExprFactor:
					case Nany::rgExprPower:
					{
						transformExprNodeToFuncCall(child); // foo < rhs, using global operators
						--i; // go back to the node before
						break;
					}

					case Nany::rgExprAssignment:
					{
						// transform all expressions 'lhs <operator> rhs' into a member func call
						// (like in 'a += b' -> 'a.+=(b)')
						assert(i > 0);
						assert(child.children.size() == 2);
						transformExprAssignmentToFuncCall(child);
						++i; // go to the next child
						break;
					}

					// transform all expressions '<op> rhs' into function calls
					case Nany::rgExprNot:
					{
						// TODO
						++i; // go to the next child
						break;
					}

					default:
					{
						++i; // go to the next child
						break;
					}
				}
			}

			// recursive call
			for (auto& child: node.children)
				normalizeExprTransformOperatorsToFuncCall(*child);
		}


		void ASTReplicator::normalizeExprReorderOperators(Nany::Node& node)
		{
			bool canReorder;
			switch (node.rule)
			{
				case rgCall:
				case rgIntrinsic: canReorder = false; break;
				default: canReorder = true;
			}

			if (canReorder)
			{
				// re-ordering nodes for operators +, *, == ...
				// this step is mandatory to have understanding AST
				for (uint i = 0; i < (uint) node.children.size(); ++i)
				{
					switch (node.children[i]->rule)
					{
						case Nany::rgExprAdd:
						case Nany::rgExprStream:
						case Nany::rgExprComparison:
						case Nany::rgExprLogic:
						case Nany::rgExprLogicAnd:
						case Nany::rgExprFactor:
						case Nany::rgExprPower:
						case Nany::rgExprNot:
						{
							if (i > 0)
							{
								auto& previous = *(node.children[i - 1]);
								switch (previous.rule)
								{
									default:
									{
										ast.nodeReparentAtTheBegining(previous, node, i - 1, *(node.children[i]));
										--i;
										break;
									}

									// do not re-parent nodes related to the structure of expressions
									// ('operators as exceptions')
									case Nany::rgOperatorAll:
									case Nany::rgOperatorKind:
									case Nany::rgOperatorAdd:
									case Nany::rgOperatorAssignment:
									case Nany::rgOperatorComparison:
									case Nany::rgOperatorFactor:
									case Nany::rgOperatorLogic:
									case Nany::rgOperatorLogicAnd:
									case Nany::rgOperatorPower:
									case Nany::rgOperatorNot:
									case Nany::rgOperatorStream:
										break;
								}
							}
							break;
						}

						default:
							break;
					}
				}


				// go for children, the container may change between each iteration
				for (uint i = 0; i < (uint) node.children.size(); ++i)
				{
					Nany::Node& child = *(node.children[i]);

					if (child.rule == rgIdentifier)
					{
						// try to rearrange the following pattern:
						// - identifier
						//    - call [for example]
						// - expr-sub-dot
						//    - identifier
						//
						// into
						//
						// - identifier
						//    - call
						//       - expr-sub-dot
						//          - identifier
						if (i + 1 < (uint) node.children.size()) // another node after the current one ?
						{
							auto& nextChild = *(node.children[i + 1]);
							auto nrule = nextChild.rule;
							// see expr-continuation

							switch (nrule)
							{
								case rgExprSubDot:
								case rgTypeSubDot:
								case rgCall:
								case rgExprTemplate:
								case rgExprSubArray:
									ast.nodeReparentAtTheEnd(nextChild, node, i + 1, child);
									break;
								default:
									break;
							}
						}
					}
				}
			}

			// recursive call
			for (auto& child: node.children)
				normalizeExprReorderOperators(*child);
		}



		inline void ASTReplicator::normalizeExpression(Nany::Node& node)
		{
			if (likely(pDuplicationSuccess))
			{
				normalizeExprReorderOperators(node);
				normalizeExprTransformOperatorsToFuncCall(node);
			}
		}



		bool ASTReplicator::generateErrorFromErrorNode(const Nany::Node& node)
		{
			pDuplicationSuccess = false;

			auto msg = report.error() << "parse error: ";
			msg << '"';

			if (node.text.size() > 43)
				msg << AnyString{node.text, 40} << "...";
			else
				msg << node.text;

			msg << '"';
			return false;
		}


		void ASTReplicator::appendNewBoolNode(Node& parent, bool onoff)
		{
			// expr-group
			// |   new (+2)
			// |       type-decl
			// |       |   identifier: bool
			// |       call
			// |           call-parameter
			// |               expr
			// |                   identifier: __true
			auto& group   = *ast.nodeAppend(parent, rgExprGroup);
			auto& newnode = *ast.nodeAppend(group, rgNew);

			auto& typeDecl = *ast.nodeAppend(newnode, rgTypeDecl);
			auto& id       = *ast.nodeAppend(typeDecl, rgIdentifier);
			id.text = "bool";

			if (onoff)
			{
				auto& value = *ast.nodeAppend(newnode, {rgCall, rgCallParameter, rgExpr, rgIdentifier});
				value.text = "__true";
			}
		}


		bool ASTReplicator::duplicateNode(Nany::Node& parent, const Nany::Node& node)
		{
			// rule of the current node
			// [this value might be changed during the node analysis]
			auto rule = node.rule;

			// ignore all nodes representing a token (tk-*), comments as well
			if (Nany::ShouldIgnoreASTRuleForDuplication(rule))
				return true;

			if (unlikely(Nany::ASTRuleIsError(rule)))
				return generateErrorFromErrorNode(node);

			switch (rule)
			{
				default:
				{
					break;
				}

				case rgNamespace:
				{
					collectNamespace(node);
					return true; // ignore the node
				}

				case rgIdentifier:
				{
					if ((parent.rule == rgExpr or parent.rule == rgExprValue))
					{
						if (node.text == "true" or node.text == "false")
						{
							appendNewBoolNode(parent, (node.text[0] == 't'));
							return true;
						}
					}
					break;
				}

				case rgExpr:
				{
					assert(not node.children.empty());
					auto& firstChild = (((node.children.size() == 1 and node.firstChild().rule == rgExprValue)
						? node.firstChild() : node))
							.firstChild();

					switch (firstChild.rule) // avoid the creation of some useless nodes
					{
						case rgClass:
						case rgVar:
						case rgTypedef:
						case rgFor:
						case rgWhile:
						case rgDoWhile:
						case rgReturn:
							return duplicateNode(parent, firstChild);
						default:
							break; // let's continue
					}
					break;
				}

				case rgExprGroup:
				{
					// remove useless groups (obvious useless parenthesis)
					switch (node.children.size())
					{
						case 3:
						{
							if (node.children[0]->rule == rgTkParentheseOpen and node.children[2]->rule == rgTkParentheseClose)
							{
								auto& middle = *(node.children[1]);
								if (middle.rule == rgIdentifier or middle.rule == rgExprGroup or middle.rule == rgNew)
									return duplicateNode(parent, middle);
							}
							break;
						}
						case 1:
						{
							auto& firstChild = node.firstChild();
							auto  frl = firstChild.rule;
							if (frl == rgExprGroup or frl == rgIdentifier or frl == rgNew)
								return duplicateNode(parent, firstChild);
							break;
						}
					}
					break;
				}

				case rgReturnInline:
				{
					rule = rgReturn; // transform the 'return-inline' into a simple 'return'
					break;
				}

				case rgFuncBody:
				{
					assert(parent.rule == rgFunction);

					// if the function is declared using a lambda operator (func foo -> 42, the return type
					// is optional (and deduced), contrary to the other form "func foo: any {return 42; }"
					// where the return type is mandatory to determine whether return values are allowed or not.
					//
					// The AST will be normalized to avoid so many differences and a return type node
					// will be added if none has been given and if a 'return-inline' node is present.
					if (not (parent.findFirst(rgFuncReturnType) < (uint) parent.children.size()))
					{
						// the parent does not declare any return type.
						// do the body has a 'return-inline' node ?
						if (node.findFirst(rgReturnInline) < (uint) node.children.size())
						{
							// all conditions are met. No type is present but should be
							auto returnType = ast.nodeAppend(parent, {rgFuncReturnType, rgType, rgTypeDecl, rgIdentifier});
							returnType->text = "any";
						}
					}
					break;
				}
			}


			//
			// duplicating the node
			//
			auto* newNode = ast.nodeAppendAsOriginal(parent, rule);

			// transfering properties
			ast.nodeCopyOffsetText(*newNode, node);
			AST::metadata(newNode).originalNode = &node;


			switch (rule)
			{
				default:
				{
					if (not node.children.empty())
						iterateThroughChildren(node, *newNode);
					break;
				}

				case Nany::rgExpr:
				case Nany::rgExprValue:
				{
					// some expr might be statements
					if (likely(not node.children.empty()))
					{
						auto& exprnode = *newNode;
						iterateThroughChildren(node, exprnode);
						normalizeExpression(exprnode);
					}
					break;
				}

				case Nany::rgVar:
				{
					// duplicate all children
					iterateThroughChildren(node, *newNode);

					// to avoid conflicts in the grammar, 'type-decl' does not use 'expr' for declaring
					// a type but must be normalized as well
					uint32_t varTypeNode = newNode->findFirst(rgVarType);
					if (varTypeNode < newNode->children.size())
						normalizeExpression(*(newNode->children[varTypeNode]));
					break;
				}

				case rgFuncBody:
				{
					if (likely(not node.children.empty()))
						iterateThroughChildren(node, *newNode);
					break;
				}

				case rgClassBody:
				{
					if (likely(not node.children.empty()))
						iterateThroughChildren(node, *newNode);
					break;
				}
			}

			return true;
		}


		inline bool ASTReplicator::run(Nany::Node& fileRootnode, Node& newroot)
		{
			bool success = true;
			fileRootnode.each([&] (const Nany::Node& subnode) -> bool
			{
				success &= duplicateNode(newroot, subnode);
				return true;
			});

			return pDuplicationSuccess and success;
		}


	} // anonymous namespace








	bool Source::passDuplicateAndNormalizeASTWL(Logs::Report& report)
	{
		auto& buildinfo = *pBuildInfo;
		auto& parser    = buildinfo.parsing.parser;
		auto& ast       = buildinfo.parsing.ast;

		//! Reset the root node
		buildinfo.parsing.rootnode = ast.nodeCreate(Nany::rgStart);
		bool success = false;

		if (parser.root and (parser.root->rule == Nany::rgStart))
		{
			if (Config::Traces::printASTBeforeNormalize)
			{
				Clob out;
				Node::Export(out, *parser.root);
				report.trace() << "before normalization:\n" << out;
			}

			ASTReplicator cloner(ast, report);
			success = cloner.run(*parser.root, *(buildinfo.parsing.rootnode));
			// retrieve data
			buildinfo.parsing.nmspc.swap(cloner.nmspc);

			if (Config::Traces::printASTAfterNormalize)
			{
				Clob out;
				Node::Export(out, *(buildinfo.parsing.rootnode));
				report.trace() << "after normalization:\n" << out;
			}
		}
		return success;
	}



} // namespace Nany
