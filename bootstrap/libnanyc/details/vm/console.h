#pragma once
#include "details/vm/context.h"


namespace ny {
namespace vm {

struct Context;

namespace console {


//! Print a text to the console
void cout(const nyprogram_cf_t&, const AnyString&) noexcept;
//! Print a text to error console
void cerr(const nyprogram_cf_t&, const AnyString&) noexcept;
//! Print a text to the console
void cout(const Context&, const AnyString&) noexcept;
//! Print a text to error console
void cerr(const Context&, const AnyString&) noexcept;

//! Set the console text color (nycout / nycerr)
void color(const nyprogram_cf_t&, nyconsole_output_t, nyoldcolor_t) noexcept;
//! Set the console text color (nycout / nycerr)
void color(const Context&, nyconsole_output_t, nyoldcolor_t) noexcept;


void exception(const Context&, const AnyString&) noexcept;
void unknownPointer(const Context&, void* ptr, uint32_t opcodeOffset = 0, uint32_t lvid = 0) noexcept;
void assertFailed(const Context&) noexcept;
void invalidPointerSize(const Context&, void*, size_t got, size_t expected) noexcept;
void badAlloc(const Context&) noexcept;
void divisionByZero(const Context&) noexcept;
void invalidLabel(const Context&, uint32_t label, uint32_t upperLabel, uint32_t opcodeOffset = 0) noexcept;
void invalidReturnType(const Context&) noexcept;
void invalidIntrinsicParameterType(const Context&) noexcept;
void unexpectedOpcode(const Context&, const AnyString& name) noexcept;
void invalidDtor(const Context&, const Atom*) noexcept;


} // namespace console
} // namespace vm
} // namespace ny

#include "console.hxx"
