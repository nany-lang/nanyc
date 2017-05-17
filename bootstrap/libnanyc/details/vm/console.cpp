#include "details/vm/console.h"

using namespace Yuni;


namespace ny {
namespace vm {
namespace console {


void exception(const Context& context, const AnyString& string) noexcept {
	cerr(context, "\n\n");
	color(context, nycerr, nyc_red);
	cerr(context, "exception: ");
	color(context, nycerr, nyc_white);
	cerr(context, string);
	color(context, nycerr, nyc_none);
	cerr(context, "\n");
}


void unknownPointer(const Context& context, void* ptr, uint32_t opcodeOffset, uint32_t lvid) noexcept {
	ShortString128 msg; // no memory allocation
	msg << "unknown pointer ";
	if (opcodeOffset != 0) {
		msg << '%' << lvid << " = " << ptr << ", opcode offset +" << opcodeOffset;
	}
	else
		msg << ptr;
	exception(context, msg);
}


void assertFailed(const Context& context) noexcept {
	exception(context, "assertion failed");
}


void invalidPointerSize(const Context& context, void* pointer, size_t got, size_t expected) noexcept {
	ShortString128 msg;
	msg << "pointer " << pointer << " size mismatch: got " << got;
	msg << " bytes, expected " << expected << " bytes";
	exception(context, msg);
}


void badAlloc(const Context& context) noexcept {
	exception(context, "failed to allocate memory");
}


void divisionByZero(const Context& context) noexcept {
	exception(context, "division by zero");
}


void invalidLabel(const Context& context, uint32_t label, uint32_t upperLabel, uint32_t opcodeOffset) noexcept {
	ShortString256 msg;
	msg << "invalid label %" << label;
	msg << " (upper: %" << upperLabel << ')';
	if (opcodeOffset != 0)
		msg << ", opcode: +" << opcodeOffset;
	exception(context, msg);
}


void invalidReturnType(const Context& context) noexcept {
	exception(context, "intrinsic invalid return type");
}


void invalidIntrinsicParameterType(const Context& context) noexcept {
	exception(context, "intrinsic invalid parameter type");
}


void unexpectedOpcode(const Context& context, const AnyString& name) noexcept {
	ShortString64 msg;
	msg << "unexpected opcode '" << name << '\'';
	exception(context, msg);
}


void invalidDtor(const Context& context, const Atom* atom) noexcept {
	String msg;
	msg << "invalid destructor for atom '";
	if (atom)
		msg << atom->caption();
	else
		msg << "<null-pointer>";
	msg << '\'';
	exception(context, msg);
}


} // namespace console
} // namespace vm
} // namespace ny
