#include "data.h"
#include "printer.inc.hpp"
#include "../sequence.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace isa {


String print(const Sequence& ircode, const ny::ir::Instruction& instr, const ny::AtomMap* atommap) {
	String text;
	Printer<String> printer{text, ircode};
	printer.atommap = atommap;
	printer.visit(instr);
	text.trimRight();
	return text;
}


void printExtract(YString& out, const Sequence& ircode, uint32_t offset, const AtomMap* atommap) {
	uint32_t context = 6;
	uint32_t start = (offset >= context) ? (offset - context) : 0;
	uint32_t end   = offset + 2;
	Printer<String> printer{out, ircode};
	printer.atommap = atommap;
	for (uint32_t i = start; i <= end and i < ircode.opcodeCount(); ++i) {
		if (i == offset)
			out << "   > | ";
		else
			out << "     | ";
		printer.visit(ircode.at(i));
	}
	out.trimRight();
}


AnyString opname(ny::ir::isa::Op opcode) {
	switch (opcode) {
		case Op::nop:            return "nop";
		case Op::opand:          return "opand";
		case Op::opor:           return "opor";
		case Op::opxor:          return "opxor";
		case Op::opmod:          return "opmod";
		case Op::eq:             return "eq";
		case Op::neq:            return "neq";
		case Op::lt:             return "lt";
		case Op::ilt:            return "ilt";
		case Op::lte:            return "lte";
		case Op::ilte:           return "ilte";
		case Op::gt:             return "gt";
		case Op::igt:            return "igt";
		case Op::gte:            return "gte";
		case Op::igte:           return "igte";
		case Op::flt:            return "flt";
		case Op::flte:           return "flte";
		case Op::fgt:            return "fgt";
		case Op::fgte:           return "fgte";
		case Op::negation:       return "negation";
		case Op::add:            return "add";
		case Op::sub:            return "sub";
		case Op::mul:            return "mul";
		case Op::div:            return "div";
		case Op::imul:           return "imul";
		case Op::idiv:           return "idiv";
		case Op::fadd:           return "fadd";
		case Op::fsub:           return "fsub";
		case Op::fmul:           return "fmul";
		case Op::fdiv:           return "fdiv";
		case Op::storeConstant:  return "storeConstant";
		case Op::store:          return "store";
		case Op::storeText:      return "storeText";
		case Op::stackalloc:     return "stackalloc";
		case Op::jmp:            return "jmp";
		case Op::jz:             return "jz";
		case Op::jnz:            return "jnz";
		case Op::ref:            return "ref";
		case Op::unref:          return "unref";
		case Op::push:           return "push";
		case Op::call:           return "call";
		case Op::intrinsic:      return "intrinsic";
		case Op::ret:            return "ret";
		case Op::allocate:       return "allocate";
		case Op::dispose:        return "dispose";
		case Op::stacksize:      return "stacksize";
		case Op::pragma:         return "pragma";
		case Op::load_u64:       return "load_u64";
		case Op::load_u32:       return "load_u32";
		case Op::load_u8:        return "load_u8";
		case Op::store_u64:      return "store_u64";
		case Op::store_u32:      return "store_u32";
		case Op::store_u8:       return "store_u8";
		case Op::memalloc:       return "memalloc";
		case Op::memfree:        return "memfree";
		case Op::memrealloc:     return "memrealloc";
		case Op::memfill:        return "memfill";
		case Op::memcopy:        return "memcopy";
		case Op::memmove:        return "memmove";
		case Op::memcmp:         return "memcmp";
		case Op::cstrlen:        return "cstrlen";
		case Op::label:          return "label";
		case Op::opassert:       return "opassert";
		case Op::memcheckhold:   return "memcheckhold";
		case Op::identify:       return "identify";
		case Op::identifyset:    return "identifyset";
		case Op::ensureresolved: return "ensureresolved";
		case Op::commontype:     return "commontype";
		case Op::assign:         return "assign";
		case Op::self:           return "self";
		case Op::tpush:          return "tpush";
		case Op::follow:         return "follow";
		case Op::blueprint:      return "blueprint";
		case Op::classdefsizeof: return "classdefsizeof";
		case Op::fieldget:       return "fieldget";
		case Op::fieldset:       return "fieldset";
		case Op::debugfile:      return "debugfile";
		case Op::debugpos:       return "debugpos";
		case Op::namealias:      return "namealias";
		case Op::comment:        return "comment";
		case Op::scope:          return "scope";
		case Op::typeisobject:   return "typeisobject";
		case Op::qualifiers:     return "qualifiers";
		case Op::end:            return "end";
	}
	throw "internal error";
}


} // namespace isa
} // namespace ir
} // namespace ny
