#pragma once
#include <yuni/core/string.h>
#include "details/grammar/nany.h"
#include <exception>

namespace ny::ir::Producer {

struct Error: public std::exception {
	Error(const AST::Node& node, const char* = "");
	const char* what() const noexcept override { return ""; }
	const AST::Node& node;
	const yuni::String message;
};

struct AttributeError final: public Error {
	AttributeError(const AST::Node& node, const yuni::ShortString32&, const char* = "");
	yuni::ShortString32 attrname;
};

struct UnexpectedNode final: public Error {
	using Error::Error;
};

} // ny::ir::Producer
