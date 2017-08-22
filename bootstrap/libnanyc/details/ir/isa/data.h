#pragma once
#include "libnanyc.h"
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
	uint32_t opcode;

	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::eq> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::neq> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::flt> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::flte> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fgt> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fgte> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::lt> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::lte> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::ilt> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::ilte> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::gt> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::gte> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::igt> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::igte> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opand> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opor> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opxor> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opmod> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::opmodi> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::negation> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fadd> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;
	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fsub> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fmul> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fdiv> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::add> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::sub> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::mul> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::div> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::imul> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::idiv> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t lhs;
	uint32_t rhs;

	template<class T> void eachLVID(const T& c) {
		c(lvid, lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::fieldget> final {
	uint32_t opcode;
	uint32_t lvid; // dest pointer
	uint32_t self;
	uint32_t var;
	template<class T> void eachLVID(const T& c) {
		c(lvid, self);
	}
};

template<> struct Operand<ny::ir::isa::Op::fieldset> final {
	uint32_t opcode;
	uint32_t lvid; // value
	uint32_t self;
	uint32_t var;
	template<class T> void eachLVID(const T& c) {
		c(lvid, self);
	}
};

template<> struct Operand<ny::ir::isa::Op::stacksize> final {
	uint32_t opcode;
	uint32_t add;
	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::comment> final {
	uint32_t opcode;
	uint32_t text;
	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::stackalloc> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t type;    // nytype_t
	uint32_t atomid;  // atom id if (type == any)
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::storeConstant> final {
	uint32_t opcode;
	uint32_t lvid;
	union {
		uint64_t u64;
		double f64;
	} value;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::storeText> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t text;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::store> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t source;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::as> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t from;
	uint32_t convert; // CTypeConvertion
	template<class T> void eachLVID(const T& c) {
		c(lvid, from);
	}
};

template<> struct Operand<ny::ir::isa::Op::ret> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t tmplvid; // unused by nyprogram_t
	template<class T> void eachLVID(const T& c) {
		c(lvid, tmplvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::push> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t name; // if named parameter
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::tpush> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t name; // if named parameter
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::call> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptr2func; // or atomid
	uint32_t instanceid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptr2func);
	}
};

template<> struct Operand<ny::ir::isa::Op::intrinsic> final {
	uint32_t opcode;
	uint32_t lvid;
	//! Intrinsic ID (only valid at execution)
	uint32_t iid;
	// intrinsic name
	uint32_t intrinsic;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::debugfile> final {
	uint32_t opcode;
	uint32_t filename;
	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::namealias> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t name;
	uint32_t forceNonSynthetic;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::debugpos> final {
	uint32_t opcode;
	uint32_t line;
	uint32_t offset;
	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::scope> final {
	uint32_t opcode;
	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::end> final {
	uint32_t opcode;
	template<class T> void eachLVID(const T&) {}
};

template<> struct Operand<ny::ir::isa::Op::qualifiers> final {
	uint32_t opcode;
	uint32_t lvid;
	TypeQualifier qualifier;
	uint32_t flag;

	template<class T> void eachLVID(const T& c) {
		static_assert(sizeof(Operand<ny::ir::isa::Op::qualifiers>) <= sizeof(uint32_t) * 4, "alignment required");
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::opassert> final {
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::memcheckhold> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t size;
	template<class T> void eachLVID(const T& c) {
		c(lvid, size);
	}
};

template<> struct Operand<ny::ir::isa::Op::label> final {
	uint32_t opcode;
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(label);
	}
};

template<> struct Operand<ny::ir::isa::Op::jmp> final {
	uint32_t opcode;
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(label);
	}
};

template<> struct Operand<ny::ir::isa::Op::jz> final {
	uint32_t opcode;
	uint32_t lvid;   // the local variable
	uint32_t result; // local variable to set to 1 if jump
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(lvid, result, label);
	}
};

template<> struct Operand<ny::ir::isa::Op::jnz> final {
	uint32_t opcode;
	uint32_t lvid;   // the local variable
	uint32_t result; // local variable to set to 1 if jump
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(lvid, result, label);
	}
};

template<> struct Operand<ny::ir::isa::Op::jzraise> final {
	uint32_t opcode;
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(label);
	}
};

template<> struct Operand<ny::ir::isa::Op::jmperrhandler> final {
	uint32_t opcode;
	uint32_t atomid;
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(label);
	}
};

template<> struct Operand<ny::ir::isa::Op::memalloc> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t regsize;
	template<class T> void eachLVID(const T& c) {
		c(lvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memfree> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t regsize;
	template<class T> void eachLVID(const T& c) {
		c(lvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memfill> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t regsize;
	uint32_t pattern;
	template<class T> void eachLVID(const T& c) {
		c(lvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memcopy> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t srclvid;
	uint32_t regsize;
	template<class T> void eachLVID(const T& c) {
		c(lvid, srclvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memmove> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t srclvid;
	uint32_t regsize;
	template<class T> void eachLVID(const T& c) {
		c(lvid, srclvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::memcmp> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t srclvid;
	uint32_t regsize; // and result as well
	template<class T> void eachLVID(const T& c) {
		c(lvid, srclvid, regsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::cstrlen> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t bits;
	uint32_t ptr;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptr);
	}
};

template<> struct Operand<ny::ir::isa::Op::load_u64> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::load_u32> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::load_u8> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptrlvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::store_u64> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::store_u32> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptrlvid);
	}
};
template<> struct Operand<ny::ir::isa::Op::store_u8> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t ptrlvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, ptrlvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::memrealloc> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t oldsize;
	uint32_t newsize;
	template<class T> void eachLVID(const T& c) {
		c(lvid, oldsize, newsize);
	}
};

template<> struct Operand<ny::ir::isa::Op::pragma> final {
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

	template<class T> void eachLVID(const T& c) {
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
	template<class T> void eachLVID(const T& c) {
		static_assert(sizeof(Operand<ny::ir::isa::Op::blueprint>) <= sizeof(uint32_t) * 4, "alignment required");
		uint32_t cplvid = lvid;
		c(cplvid);
		setLVID(cplvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::self> final {
	uint32_t opcode;
	uint32_t self;
	template<class T> void eachLVID(const T& c) {
		c(self);
	}
};

template<> struct Operand<ny::ir::isa::Op::identify> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t self;
	uint32_t text;
	template<class T> void eachLVID(const T& c) {
		c(lvid, self);
	}
};
template<> struct Operand<ny::ir::isa::Op::identifyset> final { // MUST be identical to 'identify'
	uint32_t opcode;
	uint32_t lvid;
	uint32_t self;
	uint32_t text;
	template<class T> void eachLVID(const T& c) {
		c(lvid, self);
	}
};

template<> struct Operand<ny::ir::isa::Op::ensureresolved> final {
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::commontype> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t previous;
	template<class T> void eachLVID(const T& c) {
		c(lvid, previous);
	}
};

template<> struct Operand<ny::ir::isa::Op::assign> final {
	uint32_t opcode;
	uint32_t lhs;
	uint32_t rhs;
	uint32_t disposelhs;
	template<class T> void eachLVID(const T& c) {
		c(lhs, rhs);
	}
};

template<> struct Operand<ny::ir::isa::Op::follow> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t follower;
	uint32_t symlink;
	template<class T> void eachLVID(const T& c) {
		c(lvid, follower);
	}
};

template<> struct Operand<ny::ir::isa::Op::typeisobject> final {
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::classdefsizeof> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t type; // lvid or atomid
	template<class T> void eachLVID(const T& c) {
		c(lvid, type);
	}
};

template<> struct Operand<ny::ir::isa::Op::ref> final {
	uint32_t opcode;
	uint32_t lvid;
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::unref> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t atomid;

	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::allocate> final {
	uint32_t opcode;
	uint32_t lvid;
	// atomid
	uint32_t atomid; // or atomid
	template<class T> void eachLVID(const T& c) {
		c(lvid);
	}
};

template<> struct Operand<ny::ir::isa::Op::onscopefail> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t label;
	template<class T> void eachLVID(const T& c) {
		c(lvid, label);
	}
};

template<> struct Operand<ny::ir::isa::Op::raise> final {
	uint32_t opcode;
	uint32_t lvid;
	uint32_t label;
	uint32_t atomid;
	template<class T> void eachLVID(const T& c) {
		c(lvid, label);
	}
};


AnyString opname(ny::ir::isa::Op opcode);

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
