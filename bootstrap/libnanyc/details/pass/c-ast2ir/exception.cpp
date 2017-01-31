#include "exception.h"

using namespace yuni;


namespace ny {
namespace ir {
namespace Producer {


Error::Error(const AST::Node& node, const char* msg)
	: node(node)
	, message(msg) {
}


AttributeError::AttributeError(const AST::Node& node, const yuni::ShortString32& attrname, const char* msg)
	: Error(node, msg)
	, attrname(attrname) {
}


} // namespace Producer
} // namespace ir
} // namespace ny
