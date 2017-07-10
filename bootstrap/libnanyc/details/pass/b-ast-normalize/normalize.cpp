#include "normalize.h"
#include "details/ast/ast.h"
#include "details/reporting/report.h"
#include "details/grammar/nany.h"
#include "details/atom/visibility.h"
#include "libnanyc-config.h"
#include "libnanyc-traces.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include <functional>
#include <deque>

using namespace Yuni;


namespace ny {
namespace compiler {
namespace {

inline AST::Node* nodeAppend(AST::Node& parent, enum AST::Rule rule) {
	auto* node = new AST::Node(rule);
	node->offset = parent.offset;
	node->offsetEnd = parent.offsetEnd;
	node->parent = &parent;
	parent.children.push_back(node);
	return node;
}

inline AST::Node* nodeAppend(AST::Node& parent, std::initializer_list<enum AST::Rule> list) {
	AST::Node* node = &parent;
	for (auto it : list)
		node = nodeAppend(*node, it);
	return node;
}

struct ASTReplicator final {
	explicit ASTReplicator(Logs::Report);

	bool run(AST::Node& fileRootnode, AST::Node& newroot);

public:
	//! File namespace (alias)
	std::pair<YString, AST::Node*> nmspc;

private:
	bool duplicateNode(AST::Node& out, AST::Node& node);
	void collectNamespace(const AST::Node& node);
	bool generateErrorFromErrorNode(const AST::Node& node);
	void normalizeExpression(AST::Node& node);
	void normalizeExprReorderOperators(AST::Node& node);
	void normalizeExprTransformOperatorsToFuncCall(AST::Node& node);
	void transformExprNodeToFuncCall(AST::Node& node);
	void transformExprNodeToFuncCallNOT(AST::Node& node);
	void transformExprAssignmentToFuncCall(AST::Node& node);

	void appendNewBoolNode(AST::Node& parent, bool onoff);

private:
	Logs::Report report;
	String errmsg;
	bool pDuplicationSuccess = true;

}; // class ASTReplicator


ASTReplicator::ASTReplicator(Logs::Report report)
	: report(report) {
}


void ASTReplicator::collectNamespace(const AST::Node& node) {
	auto* entity = node.xpath({AST::rgEntity});
	assert(!!entity and "mismatch grammar");
	nmspc.first.clear();
	nmspc.second = const_cast<AST::Node*>(&node);
	entity->extractChildText(nmspc.first, ny::AST::rgIdentifier, ".");
	uint32_t depth = nmspc.first.countChar('.');
	if (depth + 1 >= config::maxNamespaceDepth) {
		report.error() << "too many namespaces";
		pDuplicationSuccess = false;
		nmspc.first = "__error__";
	}
}


void ASTReplicator::transformExprNodeToFuncCallNOT(AST::Node& node) {
	// AST structure: not EXPR
	//
	// - expr-comparison
	//     - operator-not
	//     - EXPR
	//
	assert(node.children.size() == 2);
	uint32_t lhsIndex = 1;
	//  - identifier: ^not
	//      - call
	//           - call-parameter
	//                - a
	//
	// promote the current node before any changes within the subtree
	ny::AST::nodeRulePromote(node, AST::rgIdentifier);
	node.text = "^not";
	// create a new subtree
	auto call = make_ref<AST::Node>(AST::rgCall);
	auto& lhs  = *nodeAppend(*call, {AST::rgCallParameter, AST::rgExpr});
	// re-parent lhs
	ny::AST::nodeReparentAtTheEnd(node.children[lhsIndex], node, lhsIndex, lhs);
	// remove all remaining nodes
	node.children.clear();
	// re-parent the new node 'call'
	call->parent = &node;
	node.children.push_back(call);
}


void ASTReplicator::transformExprNodeToFuncCall(AST::Node& node) {
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
	AnyString opname = normalizeOperatorName(node.children[opIndex].text);
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
	ny::AST::nodeRulePromote(node, AST::rgIdentifier);
	node.text = opname;
	// create a new subtree
	auto call = make_ref<AST::Node>(AST::rgCall);
	auto& lhs = *nodeAppend(*call, {AST::rgCallParameter, AST::rgExpr});
	auto& rhs = *nodeAppend(*call, {AST::rgCallParameter, AST::rgExpr});
	// re-parent rhs first, otherwise the index will be invalidated
	ny::AST::nodeReparentAtTheEnd(node.children[rhsIndex], node, rhsIndex, rhs);
	// re-parent lhs
	ny::AST::nodeReparentAtTheEnd(node.children[lhsIndex], node, lhsIndex, lhs);
	// remove all remaining nodes
	node.children.clear();
	// re-parent the new node 'call'
	call->parent = &node;
	node.children.push_back(call);
}


void ASTReplicator::transformExprAssignmentToFuncCall(AST::Node& node) {
	// AST structure: foo() += expr; - or anything that should be asked to the object itself
	//
	//  - lhs identifier (arbitrary example)
	//      - call
	//  - expr-assignment ................ [&node]
	//      - operator-assignment: +=, -=...
	//      - rhs
	//
	assert(node.children.size() == 2);
	auto& operatorNode = node.children.front();
	uint32_t rhsIndex = 1;
	auto& rhs = node.children[rhsIndex];
	assert(operatorNode.rule == AST::rgOperatorAssignment);
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
	ny::AST::nodeRulePromote(node, AST::rgExprSubDot);
	ny::AST::nodeRulePromote(operatorNode, AST::rgIdentifier);
	// normalizing the operator
	operatorNode.text = normalizeOperatorName(operatorNode.text);
	auto& call = *nodeAppend(operatorNode, AST::rgCall);
	auto& expr = *nodeAppend(call, {AST::rgCallParameter, AST::rgExpr});
	ny::AST::nodeReparentAtTheEnd(rhs, node, rhsIndex, expr);
}


void ASTReplicator::normalizeExprTransformOperatorsToFuncCall(AST::Node& node) {
	// go for children, the container may change between each iteration
	for (uint32_t i = 0; i < node.children.size(); ) {
		AST::Node& child = node.children[i];
		switch (child.rule) {
			// transform all expressions 'lhs <operator> rhs' into a global func call
			// (like in 'a < b' -> '<(a, b)')
			case AST::rgExprAdd:
			case AST::rgExprStream:
			case AST::rgExprComparison:
			case AST::rgExprLogic:
			case AST::rgExprLogicAnd:
			case AST::rgExprFactor:
			case AST::rgExprPower: {
				transformExprNodeToFuncCall(child); // foo < rhs, using global operators
				--i; // go back to the node before
				break;
			}
			case AST::rgExprAssignment: {
				// transform all expressions 'lhs <operator> rhs' into a member func call
				// (like in 'a += b' -> 'a.+=(b)')
				assert(i > 0);
				assert(child.children.size() == 2);
				transformExprAssignmentToFuncCall(child);
				++i; // go to the next child
				break;
			}
			// transform all expressions '<op> rhs' into function calls
			case AST::rgExprNot: {
				transformExprNodeToFuncCallNOT(child);
				--i; // go to the next child
				break;
			}
			default: {
				++i; // go to the next child
				break;
			}
		}
	}
	// recursive call
	for (auto& child : node.children)
		normalizeExprTransformOperatorsToFuncCall(child);
}


void ASTReplicator::normalizeExprReorderOperators(AST::Node& node) {
	switch (node.rule) {
		case AST::rgCall:
		case AST::rgIntrinsic: {
			// do not reorder
			break;
		}
		default: {
			// re-ordering nodes for operators +, *, == ...
			// this step is mandatory to have understanding AST
			uint32_t count = node.children.size();
			for (uint32_t i = 0; i != count; ++i) {
				switch (node.children[i].rule) {
					case AST::rgExprAdd:
					case AST::rgExprStream:
					case AST::rgExprComparison:
					case AST::rgExprLogic:
					case AST::rgExprLogicAnd:
					case AST::rgExprFactor:
					case AST::rgExprPower:
					case AST::rgExprNot: {
						if (i > 0) {
							auto& previous = node.children[i - 1];
							switch (previous.rule) {
								default: {
									ny::AST::nodeReparentAtTheBegining(previous, node, i - 1, node.children[i]);
									--i;
									count = node.children.size();
									break;
								}
								// do not re-parent nodes related to the structure of expressions
								// ('operators as exceptions')
								case AST::rgOperatorAll:
								case AST::rgOperatorKind:
								case AST::rgOperatorAdd:
								case AST::rgOperatorAssignment:
								case AST::rgOperatorComparison:
								case AST::rgOperatorFactor:
								case AST::rgOperatorLogic:
								case AST::rgOperatorLogicAnd:
								case AST::rgOperatorPower:
								case AST::rgOperatorNot:
								case AST::rgOperatorStream:
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
			count = node.children.size();
			for (uint32_t i = 0; i < count; ++i) {
				AST::Node& child = node.children[i];
				if (child.rule == AST::rgIdentifier) {
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
					if (i + 1 < count) { // another node after the current one ?
						auto& nextChild = node.children[i + 1];
						auto nrule = nextChild.rule;
						// see expr-continuation
						switch (nrule) {
							case AST::rgExprSubDot:
							case AST::rgTypeSubDot:
							case AST::rgCall:
							case AST::rgExprTemplate:
							case AST::rgExprSubArray:
								ny::AST::nodeReparentAtTheEnd(nextChild, node, i + 1, child);
								count = node.children.size();
								break;
							default:
								break;
						}
					}
				}
			}
			break;
		}
	}
	// recursive call
	for (auto& child : node.children)
		normalizeExprReorderOperators(child);
}


void ASTReplicator::normalizeExpression(AST::Node& node) {
	if (likely(pDuplicationSuccess)) {
		normalizeExprReorderOperators(node);
		normalizeExprTransformOperatorsToFuncCall(node);
	}
}


bool ASTReplicator::generateErrorFromErrorNode(const AST::Node& node) {
	pDuplicationSuccess = false;
	auto msg = (report.error() << "parse error: ");
	msg << '"';
	if (node.text.size() > 43)
		msg << AnyString{node.text, 40} << "...";
	else
		msg << node.text;
	msg << '"';
	return false;
}


void ASTReplicator::appendNewBoolNode(AST::Node& parent, bool onoff) {
	// expr-group
	// |   new (+2)
	// |       type-decl
	// |       |   identifier: bool
	// |       call
	// |           call-parameter
	// |               expr
	// |                   identifier: __true
	auto& group   = *nodeAppend(parent, AST::rgExprGroup);
	auto& newnode = *nodeAppend(group, AST::rgNew);
	auto& typeDecl = *nodeAppend(newnode, AST::rgTypeDecl);
	auto& id       = *nodeAppend(typeDecl, AST::rgIdentifier);
	id.text = "bool";
	if (onoff) {
		auto& value =
			*nodeAppend(newnode, {AST::rgCall, AST::rgCallParameter, AST::rgExpr, AST::rgIdentifier});
		value.text = "__true";
	}
}


bool ASTReplicator::duplicateNode(AST::Node& parent, AST::Node& node) {
	// rule of the current node
	// [this value might be changed during the node analysis]
	auto rule = node.rule;
	// ignore all nodes representing a token (tk-*), comments as well
	if (AST::shouldIgnoreForDuplication(rule))
		return true;
	if (unlikely(AST::ruleIsError(rule)))
		return generateErrorFromErrorNode(node);
	switch (rule) {
		default: {
			break;
		}
		case AST::rgIdentifier: {
			switch (parent.rule) {
				case AST::rgExprValue:
				case AST::rgExpr: {
					if (node.text == "true" or node.text == "false") {
						appendNewBoolNode(parent, (node.text[0] == 't'));
						return true;
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		case AST::rgExpr: {
			assert(not node.children.empty());
			auto& firstChild = (((node.children.size() == 1 and node.firstChild().rule == AST::rgExprValue)
				? node.firstChild() : node)).firstChild();
			switch (firstChild.rule) { // avoid the creation of some useless nodes
				case AST::rgClass:
				case AST::rgVar:
				case AST::rgTypedef:
				case AST::rgFor:
				case AST::rgWhile:
				case AST::rgDoWhile:
				case AST::rgRaise:
				case AST::rgReturn:
				case AST::rgOn:
					return duplicateNode(parent, firstChild);
				default:
					break; // let's continue
			}
			break;
		}
		case AST::rgExprGroup: {
			// remove useless groups (obvious useless parenthesis)
			switch (node.children.size()) {
				case 3: {
					if (node.children[0].rule == AST::rgTkParentheseOpen and node.children[2].rule == AST::rgTkParentheseClose) {
						auto& middle = node.children[1];
						switch (middle.rule) {
							case AST::rgIdentifier:
							case AST::rgExprGroup:
							case AST::rgNew:
								return duplicateNode(parent, middle);
							default:
								break;
						}
					}
					break;
				}
				case 1: {
					auto& firstChild = node.firstChild();
					switch (firstChild.rule) {
						case AST::rgIdentifier:
						case AST::rgExprGroup:
						case AST::rgNew:
							return duplicateNode(parent, firstChild);
						default:
							break;
					}
					break;
				}
			}
			break;
		}
		case AST::rgReturnInline: {
			rule = AST::rgReturn; // transform the 'return-inline' into a simple 'return'
			break;
		}
		case AST::rgFuncBody: {
			assert(parent.rule == AST::rgFunction);
			// if the function is declared using a lambda operator (func foo -> 42, the return type
			// is optional (and deduced), contrary to the other form "func foo: any {return 42; }"
			// where the return type is mandatory to determine whether return values are allowed or not.
			//
			// The AST will be normalized to avoid so many differences and a return type node
			// will be added if none has been given and if a 'return-inline' node is present.
			if (not (parent.findFirst(AST::rgFuncReturnType) < parent.children.size())) {
				// the parent does not declare any return type.
				// do the body has a 'return-inline' node ?
				if (node.findFirst(AST::rgReturnInline) < node.children.size()) {
					// all conditions are met. No type is present but should be
					auto returnType =
						nodeAppend(parent, {AST::rgFuncReturnType, AST::rgType, AST::rgTypeDecl, AST::rgIdentifier});
					returnType->text = "any";
				}
			}
			break;
		}
		case AST::rgNamespace: {
			collectNamespace(node);
			return true; // ignore the node
		}
	}
	//
	// duplicating the node
	//
	auto* newNode = nodeAppend(parent, rule);
	ny::AST::nodeCopyOffsetText(*newNode, node);
	uint32_t count = node.children.size();
	for (uint32_t i = 0; i != count; ++i)
		duplicateNode(*newNode, node.children[i]);
	switch (rule) {
		default: {
			break;
		}
		case AST::rgExpr:
		case AST::rgExprValue: {
			// some expr might be statements
			normalizeExpression(*newNode);
			node.children.clear();
			break;
		}
		case AST::rgVar: {
			// to avoid conflicts in the grammar, 'type-decl' does not use 'expr' for declaring
			// a type but must be normalized as well
			uint32_t varTypeNode = newNode->findFirst(AST::rgVarType);
			if (varTypeNode < newNode->children.size())
				normalizeExpression(newNode->children[varTypeNode]);
			break;
		}
	}
	return true;
}


bool ASTReplicator::run(AST::Node& fileRootnode, AST::Node& newroot) {
	bool success = true;
	fileRootnode.each([&] (AST::Node& subnode) -> bool {
		success &= duplicateNode(newroot, subnode);
		return true;
	});
	return pDuplicationSuccess and success;
}


void dumpAST(Logs::Report& report, const AST::Node& node, const char* text) {
	Clob out;
	AST::Node::Export(out, node);
	report.trace() << text << '\n' << out;
}

} // anonymous namespace

bool passDuplicateAndNormalizeAST(ny::compiler::Source& source, Logs::Report& report) {
	auto& parser = source.parsing.parser;
	//! Reset the root node
	source.parsing.rootnode = make_ref<AST::Node>(AST::rgStart);
	if (!parser.root or (parser.root->rule != AST::rgStart))
		return false;
	if (config::traces::astBeforeNormalize)
		dumpAST(report, *parser.root, "before normalization");
	ASTReplicator cloner(report);
	bool success = cloner.run(*parser.root, *(source.parsing.rootnode));
	// retrieve data
	source.parsing.nmspc.swap(cloner.nmspc);
	if (config::traces::astAfterNormalize)
		dumpAST(report, *(source.parsing.rootnode), "after normalization");
	return success;
}

} // namespace compiler
} // namespace ny
