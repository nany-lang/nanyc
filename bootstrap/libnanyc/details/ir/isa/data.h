#pragma once
#include "libnanyc.h"
#include <nany/nany.h>
#include "opcodes.h"
#include <iosfwd>
#include <yuni/core/string.h>


namespace ny {
struct AtomMap;
namespace ir {
struct Sequence;
struct Instruction;
}
}

namespace ny {
namespace ir {
namespace isa {

enum class Pragma : uint32_t {
	//! Unknown pragma / invalid
	unknown = 0,
	//! Code generation flag
	codegen,

	// --- pragma for compilation only
	//! Size of the blueprint, in opcodes
	blueprintsize,
	//! Visibility modifier
	visibility,
	//! body start,
	bodystart,
	//! shortcircuit attribute
	shortcircuit,
	//! shortcircuit sequence offset of 'nop' instructions
	shortcircuitOpNopOffset,
	//! Shortcircuit mutate '__bool' to 'bool'
	shortcircuitMutateToBool,
	//! Builtin alias
	builtinalias,
	//! Suggestion (for error reporting)
	suggest,
	//! Set / unset an object as synthetic
	synthetic,
};
static const constexpr uint32_t PragmaCount = 1 + (uint32_t) Pragma::synthetic;


/*!
** \internal the total number of items must currently be < 2^4 (see data struct)
** \see struct Operand<isa::Op::blueprint>
*/
enum class Blueprint : uint32_t {
	//! Function definition
	funcdef,
	//! Variable member definition
	vardef,
	//! Class definition
	classdef,
	//! Typedef
	typealias,
	//! Parameter definition (for a function)
	param,
	//! Template parameter
	gentypeparam,
	//! Parameter definition, with auto assignment
	paramself,
	//! Namespace definition (one part of it)
	namespacedef,
	//! Unit (source file)
	unit,

}; // enum Blueprint

enum class TypeQualifier : uint32_t {
	//! Ref qualifier
	ref,
	//! 'const' qualifier
	constant,
};
static const constexpr uint32_t TypeQualifierCount = 1 + (uint32_t) TypeQualifier::constant;


template<ny::ir::isa::Op O> struct Operand final {};


template<> struct Operand<ny::ir::isa::Op::nop> final {
	constexpr static const char* opname() {
		return "nop";
	}
	uint32_t opcode;

	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::eq> final {
	constexpr static const char* opname() {
		return "eq";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::neq> final {
	constexpr static const char* opname() {
		return "neq";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::flt> final {
	constexpr static const char* opname() {
		return "flt";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::flte> final {
	constexpr static const char* opname() {
		return "flte";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fgt> final {
	constexpr static const char* opname() {
		return "fgt";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fgte> final {
	constexpr static const char* opname() {
		return "fgte";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::lt> final {
	constexpr static const char* opname() {
		return "lt";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::lte> final {
	constexpr static const char* opname() {
		return "lte";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::ilt> final {
	constexpr static const char* opname() {
		return "ilt";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::ilte> final {
	constexpr static const char* opname() {
		return "ilte";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::gt> final {
	constexpr static const char* opname() {
		return "gt";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::gte> final {
	constexpr static const char* opname() {
		return "gte";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::igt> final {
	constexpr static const char* opname() {
		return "igt";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::igte> final {
	constexpr static const char* opname() {
		return "igte";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opand> final {
	constexpr static const char* opname() {
		return "and";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opor> final {
	constexpr static const char* opname() {
		return "or";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opxor> final {
	constexpr static const char* opname() {
		return "xor";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opmod> final {
	constexpr static const char* opname() {
		return "mod";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::negation> final {
	constexpr static const char* opname() {
		return "negation";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fadd> final {
	constexpr static const char* opname() {
		return "fadd";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;
	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fsub> final {
	constexpr static const char* opname() {
		return "fsub";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fmul> final {
	constexpr static const char* opname() {
		return "fmul";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fdiv> final {
	constexpr static const char* opname() {
		return "fdiv";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::add> final {
	constexpr static const char* opname() {
		return "add";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::sub> final {
	constexpr static const char* opname() {
		return "sub";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::mul> final {
	constexpr static const char* opname() {
		return "mul";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::div> final {
	constexpr static const char* opname() {
		return "div";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::imul> final {
	constexpr static const char* opname() {
		return "imul";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::idiv> final {
	constexpr static const char* opname() {
		return "idiv";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fieldget> final {
	constexpr static const char* opname() {
		return "fieldget";
	}
	uint32_t opcode;
	uint32_t lvid; // dest pointer
	uint32_t self;
	uint32_t var;
	template<class T> void eachLVID(T& c) {
		c(lvid, self);
	}
};

template<> struct Operand<ny::ir::isa::Op::fieldset> final {
	constexpr static const char* opname() {
		return "fieldset";
	}
	uint32_t opcode;
	uint32_t lvid; // value
	uint32_t self;
	uint32_t var;
	template<class T> void eachLVID(T& c) {
		c(lvid, self);
	}
};

template<> struct Operand<ny::ir::isa::Op::stacksize> final {
	constexpr static const char* opname() {
		return "stacksize";
	}
	uint32_t opcode;
	uint32_t add;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::comment> final {
	constexpr static const char* opname() {
		return "comment";
	}
	uint32_t opcode;
	uint32_t text;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::stackalloc> final {
	constexpr static const char* opname() {
		return "stackalloc";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t type;    // nytype_t
	uint32_t atomid;  // atom id if (type == any)
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::storeConstant> final {
	constexpr static const char* opname() {
		return "storeConstant";
	}
	uint32_t opcode;
	uint32_t lvid;
	union {
		uint64_t u64;
		double f64;
	} value;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::storeText> final {
	constexpr static const char* opname() {
		return "storeText";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t text;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::store> final {
	constexpr static const char* opname() {
		return "store";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t source;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::ret> final {
	constexpr static const char* opname() {
		return "ret";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t tmplvid; // unused by nyprogram_t
	template<class T> void eachLVID(T& c) {
		c(lvid, tmplvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::push> final {
	constexpr static const char* opname() {
		return "push";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t name; // if named parameter
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::tpush> final {
	constexpr static const char* opname() {
		return "tpush";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t name; // if named parameter
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::call> final {
	constexpr static const char* opname() {
		return "call";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptr2func; // or atomid
	uint32_t instanceid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptr2func);
	}
};

template<> struct Operand<ny::ir::isa::Op::intrinsic> final {
	constexpr static const char* opname() {
		return "intrinsic";
	}
	uint32_t opcode;
	uint32_t lvid;
	//! Intrinsic ID (only valid at execution)
	uint32_t iid;
	// intrinsic name
	uint32_t intrinsic;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::debugfile> final {
	constexpr static const char* opname() {
		return "debugfile";
	}
	uint32_t opcode;
	uint32_t filename;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::namealias> final {
	constexpr static const char* opname() {
		return "namealias";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t name;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::debugpos> final {
	constexpr static const char* opname() {
		return "debugpos";
	}
	uint32_t opcode;
	uint32_t line;
	uint32_t offset;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::scope> final {
	constexpr static const char* opname() {
		return "scope";
	}
	uint32_t opcode;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::end> final {
	constexpr static const char* opname() {
		return "end";
	}
	uint32_t opcode;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::qualifiers> final {
	constexpr static const char* opname() {
		return "qualifiers";
	}
	uint32_t opcode;
	uint32_t lvid;
	TypeQualifier qualifier;
	uint32_t flag;

	template<class T> void eachLVID(T& c) {
		static_assert(sizeof(Operand<ny::ir::isa::Op::qualifiers>) <= sizeof(uint32_t) * 4, "alignment required");
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::opassert> final {
	constexpr static const char* opname() {
		return "assert";
	}
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::memcheckhold> final {
	constexpr static const char* opname() {
		return "memcheckhold";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t size;
	template<class T> void eachLVID(T& c) {
		c(lvid, size);
	}
};

template<> struct Operand<ny::ir::isa::Op::label> final {
	constexpr static const char* opname() {
		return "label";
	}
	uint32_t opcode;
	uint32_t label;
	template<class T> void eachLVID(T& c) {
		c(label);
	}
};

template<> struct Operand<ny::ir::isa::Op::jmp> final {
	constexpr static const char* opname() {
		return "jmp";
	}
	uint32_t opcode;
	uint32_t label;
	template<class T> void eachLVID(T&) {}
};

template<> struct Operand<ny::ir::isa::Op::jz> final {
	constexpr static const char* opname() {
		return "jz";
	}
	uint32_t opcode;
	uint32_t lvid;   // the local variable
	uint32_t result; // local variable to set to 1 if jump
	uint32_t label;
	template<class T> void eachLVID(T& c) {
		c(lvid, result);
	}
};

template<> struct Operand<ny::ir::isa::Op::jnz> final {
	constexpr static const char* opname() {
		return "jnz";
	}
	uint32_t opcode;
	uint32_t lvid;   // the local variable
	uint32_t result; // local variable to set to 1 if jump
	uint32_t label;
	template<class T> void eachLVID(T& c) {
		c(lvid, result);
	}
};

template<> struct Operand<ny::ir::isa::Op::memalloc> final {
	constexpr static const char* opname() {
		return "memalloc";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t regsize;
	template<class T> void eachLVID(T& c) {
		c(lvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memfree> final {
	constexpr static const char* opname() {
		return "memfree";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t regsize;
	template<class T> void eachLVID(T& c) {
		c(lvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memfill> final {
	constexpr static const char* opname() {
		return "memfill";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t regsize;
	uint32_t pattern;
	template<class T> void eachLVID(T& c) {
		c(lvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memcopy> final {
	constexpr static const char* opname() {
		return "memcopy";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t srclvid;
	uint32_t regsize;
	template<class T> void eachLVID(T& c) {
		c(lvid, srclvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memmove> final {
	constexpr static const char* opname() {
		return "memmove";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t srclvid;
	uint32_t regsize;
	template<class T> void eachLVID(T& c) {
		c(lvid, srclvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memcmp> final {
	constexpr static const char* opname() {
		return "memcmp";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t srclvid;
	uint32_t regsize; // and result as well
	template<class T> void eachLVID(T& c) {
		c(lvid, srclvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::cstrlen> final {
	constexpr static const char* opname() {
		return "cstrlen";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t bits;
	uint32_t ptr;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptr);
	}
};

template<> struct Operand<ny::ir::isa::Op::load_u64> final {
	constexpr static const char* opname() {
		return "load_u64";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::load_u32> final {
	constexpr static const char* opname() {
		return "load_u32";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::load_u8> final {
	constexpr static const char* opname() {
		return "load_u8";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptrlvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::store_u64> final {
	constexpr static const char* opname() {
		return "store_u64";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::store_u32> final {
	constexpr static const char* opname() {
		return "store_u32";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::store_u8> final {
	constexpr static const char* opname() {
		return "store_u8";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(T& c) {
		c(lvid, ptrlvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::memrealloc> final {
	constexpr static const char* opname() {
		return "memrealloc";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t oldsize;
	uint32_t newsize;
	template<class T> void eachLVID(T& c) {
		c(lvid, oldsize, newsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::pragma> final {
	constexpr static const char* opname() {
		return "pragma";
	}
	uint32_t opcode;
	Pragma pragma;

	union {
		uint32_t codegen;
		uint32_t error;
		uint32_t visibility;
		uint32_t blueprintsize;
		uint32_t shortcircuit;
		uint32_t suggest;
		struct {
			uint32_t lvid;
			uint32_t onoff;
		} synthetic;
		struct {
			uint32_t namesid;
		} builtinalias;
		struct {
			uint32_t label;
		} shortcircuitMetadata;
		struct {
			uint32_t lvid;
			uint32_t source;
		} shortcircuitMutate;
	}
	value;

	template<class T> void eachLVID(T& c) {
		static_assert(sizeof(Operand<ny::ir::isa::Op::pragma>) <= sizeof(uint32_t) * 4, "alignment required");
		switch (pragma) {
			case Pragma::synthetic: {
				c(value.synthetic.lvid);
				break;
			}
			case Pragma::shortcircuitOpNopOffset: {
				c(value.shortcircuitMetadata.label);
				break;
			}
			case Pragma::shortcircuitMutateToBool: {
				c(value.shortcircuitMutate.lvid);
				break;
			}
			case Pragma::unknown:
			case Pragma::codegen:
			case Pragma::blueprintsize:
			case Pragma::visibility:
			case Pragma::bodystart:
			case Pragma::shortcircuit:
			case Pragma::builtinalias:
			case Pragma::suggest:
				break;
		}
	}
};

template<> struct Operand<ny::ir::isa::Op::blueprint> final {
	constexpr static const char* opname() {
		return "blueprint";
	}

	uint32_t opcode;
	//! Kind of blueprint (classdef, vardef, funcdef...)
	//! \see enum ir::isa::Blueprint
	uint32_t kind: 4;
	//! Attached lvid (if any)
	uint32_t lvid: 28; // should be big enough even for large func
	//! Blueprint name
	uint32_t name;
	//! Attached atomid (if any)
	uint32_t atomid;

	void setLVID(uint32_t newlvid) { // only to avoid warning
		union {
			uint32_t i;
			uint32_t lvid: 28;
		} converter {newlvid};
		lvid = converter.lvid;
	}
	template<class T> void eachLVID(T& c) {
		static_assert(sizeof(Operand<ny::ir::isa::Op::blueprint>) <= sizeof(uint32_t) * 4, "alignment required");
		uint32_t cplvid = lvid;
		c(cplvid);
		setLVID(cplvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::self> final {
	constexpr static const char* opname() {
		return "self";
	}
	uint32_t opcode;
	uint32_t self;
	template<class T> void eachLVID(T& c) {
		c(self);
	}
};

template<> struct Operand<ny::ir::isa::Op::identify> final {
	constexpr static const char* opname() {
		return "identify";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t self;
	uint32_t text;
	template<class T> void eachLVID(T& c) {
		c(lvid, self);
	}
};
template<> struct Operand<ny::ir::isa::Op::identifyset> final { // MUST be identical to 'identify'
	constexpr static const char* opname() {
		return "identifyset";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t self;
	uint32_t text;
	template<class T> void eachLVID(T& c) {
		c(lvid, self);
	}
};

template<> struct Operand<ny::ir::isa::Op::ensureresolved> final {
	constexpr static const char* opname() {
		return "ensureresolved";
	}
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::commontype> final {
	constexpr static const char* opname() {
		return "commontype";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t previous;
	template<class T> void eachLVID(T& c) {
		c(lvid, previous);
	}
};

template<> struct Operand<ny::ir::isa::Op::assign> final {
	constexpr static const char* opname() {
		return "assign";
	}
	uint32_t opcode;
	uint32_t lhs;
	uint32_t rhs;
	uint32_t disposelhs;
	template<class T> void eachLVID(T& c) {
		c(lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::follow> final {
	constexpr static const char* opname() {
		return "follow";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t follower;
	uint32_t symlink;
	template<class T> void eachLVID(T& c) {
		c(lvid, follower);
	}
};

template<> struct Operand<ny::ir::isa::Op::typeisobject> final {
	constexpr static const char* opname() {
		return "typeisobject";
	}
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::classdefsizeof> final {
	constexpr static const char* opname() {
		return "classdefsizeof";
	}
	uint32_t opcode;
	uint32_t lvid;
	uint32_t type; // lvid or atomid
	template<class T> void eachLVID(T& c) {
		c(lvid, type);
	}
};

template<> struct Operand<ny::ir::isa::Op::ref> final {
	constexpr static const char* opname() {
		return "ref";
	}
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::unref> final {
	constexpr static const char* opname() {
		return "unref";
	}
	uint32_t opcode;
	uint32_t lvid;

	// atomid
	uint32_t atomid; // or atomid
	// destructor to call
	uint32_t instanceid;

	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::allocate> final {
	constexpr static const char* opname() {
		return "allocate";
	}
	uint32_t opcode;
	uint32_t lvid;
	// atomid
	uint32_t atomid; // or atomid
	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::dispose> final {
	constexpr static const char* opname() {
		return "dispose";
	}
	uint32_t opcode;
	uint32_t lvid;

	// atomid
	uint32_t atomid; // or atomid
	// destructor to call
	uint32_t instanceid;

	template<class T> void eachLVID(T& c) {
		c(lvid);
	}
};



Yuni::String print(const Sequence&, const ny::ir::Instruction&, const AtomMap* = nullptr);

template<ny::ir::isa::Op O>
inline Yuni::String
print(const Sequence& sequence, const ny::ir::isa::Operand<O>& operands, const AtomMap* map = nullptr) {
	return print(sequence, reinterpret_cast<const ny::ir::Instruction&>(operands), map);
}

void printExtract(YString& out, const Sequence&, uint32_t offset, const AtomMap* = nullptr);


} // namespace isa
} // namespace ir
} // namespace ny
