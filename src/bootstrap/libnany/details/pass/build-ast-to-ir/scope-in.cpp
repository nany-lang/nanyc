#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprIn(const AST::Node& node, LVID& localvar)
	{
		// Example: i in a | i > 10
		//
		//in (+4)
		//	in-vars
		//	|   identifier: i
		//	tk-in
		//	in-container
		//	|   expr-value
		//	|       identifier: a
		//	in-filter (+2)
		//		tk-pipe: |
		//		expr (+2)
		//			expr-value
		//			|   identifier: i
		//			expr-comparison (+2)
		//				operator-comparison: >
		//				expr-value
		//					number
		//						number-value: 10
		//							integer: 10
		//
		// .. will be converted into ..
		//
		//
		//   expr
		//	   expr-value (+4)
		//		   tk-parenthese-open, (
		//		   expr-group
		//		   |   expr-value
		//		   |	   identifier: a
		//		   tk-parenthese-close, )
		//		   expr-sub-dot (+3)
		//			   tk-dot
		//			   identifier: makeview
		//			   call (+3)
		//				   tk-parenthese-open, (
		//				   call-parameter
		//				   |   expr
		//				   |	   expr-value
		//				   |		   new (+2)
		//				   |			   tk-new
		//				   |			   type-decl
		//				   |				   class (+2)
		//				   |					   tk-class
		//				   |					   class-body (+3)
		//				   |						   tk-brace-open, {
		//				   |						   expr
		//				   |			  expr-value
		//				   |				  function (+4)
		//				   |					  function-kind
		//				   |					  |   function-kind-operator (+2)
		//				   |					  |	   tk-operator
		//				   |					  |	   function-kind-opname: ()
		//				   |					  func-params (+3)
		//				   |					  |   tk-parenthese-open, (
		//				   |					  |   func-param (+2)
		//				   |					  |   |   cref
		//				   |					  |   |   identifier: i
		//				   |					  |   tk-parenthese-close, )
		//				   |					  func-return-type (+2)
		//				   |					  |   tk-colon
		//				   |					  |   type
		//				   |					  |	   type-qualifier
		//				   |					  |		   ref
		//				   |					  func-body
		//				   |						  return-inline (+3)
		//				   |							  tk-arrow: ->
		//				   |							  expr
		//				   |				 expr-value
		//				   |					 identifier: true


		bool success = true;
		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgInFilter:
				{
					ICE(child) << "predicate for views not implemented yet";
					break;
				}
				default:
					success = ICEUnexpectedNode(child, "[expr/in]");
			}
		}
		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
