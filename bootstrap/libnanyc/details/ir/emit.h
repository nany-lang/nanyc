#pragma once
#include "sequence.h"
#include "details/ir/isa/data.h"


namespace ny {
namespace ir {
namespace emit {
namespace {

struct IRCodeRef final {
	IRCodeRef(Sequence& ircode) : ircode(ircode) {}
	IRCodeRef(Sequence* ircode) : ircode(*ircode) { assert(ircode != nullptr); }
	Sequence& ircode;
};


inline void opnot(IRCodeRef ref, uint32_t lvid, uint32_t lhs) {
	auto& operands = ref.ircode.emit<isa::Op::negation>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
}


inline void opfadd(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::fadd>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opfsub(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::fsub>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opfdiv(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::fdiv>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opfmul(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::fmul>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opadd(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::add>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opsub(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::sub>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opdiv(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::div>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opmul(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::mul>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opimul(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::imul>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opidiv(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::idiv>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opeq(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::eq>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opneq(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::neq>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opflt(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::flt>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opflte(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::flte>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opfgt(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::fgt>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opfgte(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::fgte>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void oplt(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::lt>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void oplte(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::lte>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opilt(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::ilt>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opilte(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::ilte>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opgt(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::gt>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opgte(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::gte>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opigt(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::igt>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opigte(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::igte>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opand(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::opand>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opor(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::opor>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opxor(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::opxor>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void opmod(IRCodeRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
	auto& operands = ref.ircode.emit<isa::Op::opmod>();
	operands.lvid  = lvid;
	operands.lhs   = lhs;
	operands.rhs   = rhs;
}


inline void nop(IRCodeRef ref) {
	ref.ircode.emit<isa::Op::nop>();
}


//! Copy two register
inline void copy(IRCodeRef ref, uint32_t lvid, uint32_t source) {
	auto& operands = ref.ircode.emit<isa::Op::store>();
	operands.lvid = lvid;
	operands.source = source;
}


inline void constantu64(IRCodeRef ref, uint32_t lvid, uint64_t value) {
	auto& operands = ref.ircode.emit<isa::Op::storeConstant>();
	operands.lvid = lvid;
	operands.value.u64 = value;
}


inline void constantf64(IRCodeRef ref, uint32_t lvid, double value) {
	auto& operands = ref.ircode.emit<isa::Op::storeConstant>();
	operands.lvid  = lvid;
	operands.value.f64 = value;
}


inline void constantbool(IRCodeRef ref, uint32_t lvid, bool value) {
	constantu64(ref, lvid, value ? 1 : 0);
}


inline uint32_t constantText(IRCodeRef ref, uint32_t lvid, const AnyString& text) {
	auto& operands = ref.ircode.emit<isa::Op::storeText>();
	operands.lvid = lvid;
	operands.text = ref.ircode.stringrefs.ref(text);
	return operands.text;
}


//! Allocate a new variable on the stack and get the register
inline uint32_t alloc(IRCodeRef ref, uint32_t lvid, nytype_t type = nyt_any) {
	auto& operands  = ref.ircode.emit<isa::Op::stackalloc>();
	operands.lvid   = lvid;
	operands.type   = static_cast<uint32_t>(type);
	operands.atomid = (uint32_t) - 1;
	return lvid;
}


//! Allocate a new variable on the stack and assign a value to it and get the register
inline uint32_t allocu64(IRCodeRef ref, uint32_t lvid, nytype_t type, uint64_t value) {
	ir::emit::alloc(ref, lvid, type);
	ir::emit::constantu64(ref, lvid, value);
	return lvid;
}


//! Allocate a new variable on the stack and assign a value to it and get the register
inline uint32_t allocf64(IRCodeRef ref, uint32_t lvid, nytype_t type, double value) {
	ir::emit::alloc(ref, lvid, type);
	ir::emit::constantf64(ref, lvid, value);
	return lvid;
}


//! Allocate a new variable on the stack and assign a text to it and get the register
inline uint32_t alloctext(IRCodeRef ref, uint32_t lvid, const AnyString& text) {
	ir::emit::alloc(ref, lvid, nyt_ptr);
	ir::emit::constantText(ref, lvid, text);
	return lvid;
}


inline uint32_t increaseStacksize(IRCodeRef ref, uint32_t size = 0) {
	uint32_t offset = ref.ircode.opcodeCount();
	ref.ircode.emit<isa::Op::stacksize>().add = size;
	return offset;
}


inline void cassert(IRCodeRef ref, uint32_t lvid) {
	ref.ircode.emit<isa::Op::opassert>().lvid = lvid;
}


inline uint32_t objectAlloc(IRCodeRef ref, uint32_t lvid, uint32_t atomid) {
	auto& operands = ref.ircode.emit<isa::Op::allocate>();
	operands.lvid = lvid;
	operands.atomid = atomid;
	return lvid;
}


inline void ref(IRCodeRef ref, uint32_t lvid) {
	ref.ircode.emit<isa::Op::ref>().lvid = lvid;
}


inline void unref(IRCodeRef ref, uint32_t lvid, uint32_t atomid, uint32_t instanceid) {
	auto& operands = ref.ircode.emit<isa::Op::unref>();
	operands.lvid = lvid;
	operands.atomid = atomid;
	operands.instanceid = instanceid;
}


inline void scopeBegin(IRCodeRef ref) {
	ref.ircode.emit<isa::Op::scope>();
}


inline void scopeEnd(IRCodeRef ref) {
	ref.ircode.emit<isa::Op::end>();
}


inline uint32_t label(IRCodeRef ref, uint32_t labelid) {
	ref.ircode.emit<isa::Op::label>().label = labelid;
	return labelid;
}


//! Unconditional jump
inline void jmp(IRCodeRef ref, uint32_t label) {
	ref.ircode.emit<isa::Op::jmp>().label = label;
}


//! jump if zero
inline void jz(IRCodeRef ref, uint32_t lvid, uint32_t result, uint32_t label) {
	auto& opc  = ref.ircode.emit<isa::Op::jz>();
	opc.lvid   = lvid;
	opc.result = result;
	opc.label  = label;
}


//! jump if not zero
inline void jnz(IRCodeRef ref, uint32_t lvid, uint32_t result, uint32_t label) {
	auto& opc  = ref.ircode.emit<isa::Op::jnz>();
	opc.lvid   = lvid;
	opc.result = result;
	opc.label  = label;
}


inline void identify(IRCodeRef ref, uint32_t lvid, const AnyString& name, uint32_t self) {
	auto& operands = ref.ircode.emit<isa::Op::identify>();
	operands.lvid  = lvid;
	operands.self  = self;
	operands.text  = ref.ircode.stringrefs.ref(name);
}


inline void namealias(IRCodeRef ref, uint32_t lvid, const AnyString& name) {
	auto& operands = ref.ircode.emit<isa::Op::namealias>();
	operands.lvid  = lvid;
	operands.name  = ref.ircode.stringrefs.ref(name);
}


inline void push(IRCodeRef ref, uint32_t lvid) {
	auto& operands = ref.ircode.emit<isa::Op::push>();
	operands.lvid  = lvid;
	operands.name  = 0;
}


inline void push(IRCodeRef ref, uint32_t lvid, const AnyString& name) {
	auto& operands = ref.ircode.emit<isa::Op::push>();
	operands.lvid  = lvid;
	operands.name  = (not name.empty()) ? ref.ircode.stringrefs.ref(name) : 0;
}


inline void tpush(IRCodeRef ref, uint32_t lvid) {
	auto& operands = ref.ircode.emit<isa::Op::tpush>();
	operands.lvid  = lvid;
	operands.name  = 0;
}


inline void tpush(IRCodeRef ref, uint32_t lvid, const AnyString& name) {
	auto& operands = ref.ircode.emit<isa::Op::tpush>();
	operands.lvid  = lvid;
	operands.name  = (not name.empty()) ? ref.ircode.stringrefs.ref(name) : 0;
}


inline void call(IRCodeRef ref, uint32_t lvid, uint32_t ptr2func) {
	auto& operands = ref.ircode.emit<isa::Op::call>();
	operands.lvid  = lvid;
	operands.ptr2func = ptr2func;
	operands.instanceid = (uint32_t) - 1;
}


inline void call(IRCodeRef ref, uint32_t lvid, uint32_t atomid, uint32_t instanceid) {
	assert(instanceid != (uint32_t) - 1);
	auto& operands = ref.ircode.emit<isa::Op::call>();
	operands.lvid  = lvid;
	operands.ptr2func = atomid;
	operands.instanceid = instanceid;
}


inline void intrinsic(IRCodeRef ref, uint32_t lvid, const AnyString& name, uint32_t id = (uint32_t) - 1) {
	auto& operands     = ref.ircode.emit<isa::Op::intrinsic>();
	operands.lvid      = lvid;
	operands.intrinsic = ref.ircode.stringrefs.ref(name);
	operands.iid       = id;
}


//! Return with no value
inline void ret(IRCodeRef ref) {
	auto& operands   = ref.ircode.emit<isa::Op::ret>();
	operands.lvid    = 0;
	operands.tmplvid = 0;
}


//! Return from value
inline void ret(IRCodeRef ref, uint32_t lvid, uint32_t tmplvid) {
	auto& operands   = ref.ircode.emit<isa::Op::ret>();
	operands.lvid    = lvid;
	operands.tmplvid = tmplvid;
}


inline void raise(IRCodeRef ref, uint32_t lvid) {
	ref.ircode.emit<isa::Op::raise>().lvid = lvid;
}


inline void assign(IRCodeRef ref, uint32_t lhs, uint32_t rhs, bool canDisposeLHS) {
	auto& operands = ref.ircode.emit<isa::Op::assign>();
	operands.lhs = lhs;
	operands.rhs = rhs;
	operands.disposelhs = canDisposeLHS;
}


inline void fieldget(IRCodeRef ref, uint32_t lvid, uint32_t self, uint32_t fieldindex) {
	assert(lvid != 0 and self != 0);
	auto& operands = ref.ircode.emit<isa::Op::fieldget>();
	operands.lvid  = lvid;
	operands.self  = self;
	operands.var   = fieldindex;
}


inline void fieldset(IRCodeRef ref, uint32_t lvid, uint32_t self, uint32_t varid) {
	assert(lvid != 0 and self != 0);
	auto& operands = ref.ircode.emit<isa::Op::fieldset>();
	operands.lvid  = lvid;
	operands.self  = self;
	operands.var   = varid;
}


template<class T> struct TraceWriter final {
	static void emit(IRCodeRef ref, const T& value) {
		ref.ircode.emit<isa::Op::comment>().text = ref.ircode.stringrefs.ref(value());
	}
};


template<uint32_t N>
struct TraceWriter<char[N]> final {
	static void emit(IRCodeRef ref, const char* value) {
		ref.ircode.emit<isa::Op::comment>().text = ref.ircode.stringrefs.ref(AnyString{value, N});
	}
};


//! Insert a comment in the IR code
template<class T>
inline void trace(IRCodeRef ref, const T& value) {
	//! ir::emit::trace(out, [](){return "some comments here";});
	if (yuni::debugmode)
		TraceWriter<T>::emit(std::move(ref), value);
}


//! Insert a comment in the IR code
template<class T>
inline void trace(IRCodeRef ref, bool condition, const T& value) {
	//! ir::emit::trace(out, [](){return "some comments here";});
	if (yuni::debugmode and condition)
		TraceWriter<T>::emit(std::move(ref), value);
}


//! Insert empty comment line in the IR code
inline void trace(IRCodeRef ref) {
	if (yuni::debugmode)
		ref.ircode.emit<isa::Op::comment>().text = 0;
}


} // namespace
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace emit {
namespace on {
namespace {


inline uint32_t scopefail(IRCodeRef ref, uint32_t lvid, uint32_t label) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands = ref.ircode.emit<isa::Op::onscopefail>();
	operands.lvid  = lvid;
	operands.label = label;
	return offset;
}


} // namespace
} // namespace on
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace emit {
namespace type {
namespace {


inline void isself(IRCodeRef ref, uint32_t lvid) {
	ref.ircode.emit<isa::Op::self>().self = lvid;
}


inline void isobject(IRCodeRef ref, uint32_t lvid) {
	ref.ircode.emit<isa::Op::typeisobject>().lvid = lvid;
}


inline void common(IRCodeRef ref, uint32_t lvid, uint32_t previous) {
	auto& operands = ref.ircode.emit<isa::Op::commontype>();
	operands.lvid = lvid;
	operands.previous = previous;
}


inline void objectSizeof(IRCodeRef ref, uint32_t lvid, uint32_t type) {
	auto& operands = ref.ircode.emit<isa::Op::classdefsizeof>();
	operands.lvid  = lvid;
	operands.type  = type;
}


inline void qualifierRef(IRCodeRef ref, uint32_t lvid, bool flag) {
	auto& operands = ref.ircode.emit<isa::Op::qualifiers>();
	operands.lvid  = lvid;
	operands.flag  = static_cast<uint32_t>(flag);
	operands.qualifier = ir::isa::TypeQualifier::ref;
}


inline void qualifierConst(IRCodeRef ref, uint32_t lvid, bool flag) {
	auto& operands = ref.ircode.emit<isa::Op::qualifiers>();
	operands.lvid  = lvid;
	operands.flag  = static_cast<uint32_t>(flag);
	operands.qualifier = ir::isa::TypeQualifier::constant;
}


inline void ensureResolved(IRCodeRef ref, uint32_t lvid) {
	ref.ircode.emit<isa::Op::ensureresolved>().lvid = lvid;
}


} // namespace
} // namespace type
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace emit {
namespace blueprint {
namespace {


template<ir::isa::Blueprint KindT>
inline auto& make(IRCodeRef& ref) {
	auto& operands = ref.ircode.emit<ir::isa::Op::blueprint>();
	operands.kind = (decltype(operands.kind)) KindT;
	return operands;
}


inline uint32_t unit(IRCodeRef ref, const AnyString& filename) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::unit>(ref);
	operands.name   = ref.ircode.stringrefs.ref(filename);
	operands.atomid = (uint32_t) - 1;
	operands.lvid   = 0u;
	return offset;
}


inline void namespacedef(IRCodeRef ref, const AnyString& name) {
	auto& operands  = blueprint::make<ir::isa::Blueprint::namespacedef>(ref);
	operands.name   = ref.ircode.stringrefs.ref(name);
	operands.atomid = (uint32_t) - 1;
	operands.lvid   = 0u;
}


inline uint32_t classdef(IRCodeRef ref, uint32_t lvid) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::classdef>(ref);
	operands.name   = 0u;
	operands.atomid = (uint32_t) - 1;
	operands.setLVID(lvid);
	return offset;
}


inline void func(IRCodeRef ref, const AnyString& name, uint32_t atomid) {
	auto& operands  = blueprint::make<ir::isa::Blueprint::funcdef>(ref);
	operands.name   = ref.ircode.stringrefs.ref(name);
	operands.atomid = atomid;
	operands.lvid   = 0u;
}


inline uint32_t func(IRCodeRef ref) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::funcdef>(ref);
	operands.name   = 0u;
	operands.atomid = (uint32_t) - 1;
	operands.lvid   = 0u;
	return offset;
}


inline void var(IRCodeRef ref, uint32_t lvid, const AnyString& name) {
	auto& operands  = blueprint::make<ir::isa::Blueprint::vardef>(ref);
	operands.name   = ref.ircode.stringrefs.ref(name);
	operands.atomid = (uint32_t) - 1;
	operands.setLVID(lvid);
}


inline uint32_t typealias(IRCodeRef ref, const AnyString& name, uint32_t atomid = (uint32_t) - 1) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::typealias>(ref);
	operands.name   = ref.ircode.stringrefs.ref(name);
	operands.atomid = atomid;
	operands.lvid   = 0u;
	return offset;
}


inline uint32_t param(IRCodeRef ref, uint32_t lvid, const AnyString& name) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::param>(ref);
	operands.name   = ref.ircode.stringrefs.ref(name);
	operands.atomid = (uint32_t) - 1;
	operands.setLVID(lvid);
	return offset;
}


inline uint32_t param(IRCodeRef ref, uint32_t lvid) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::param>(ref);
	operands.name   = 0;
	operands.atomid = (uint32_t) - 1;
	operands.setLVID(lvid);
	return offset;
}


inline uint32_t tparam(IRCodeRef ref, uint32_t lvid, const AnyString& name) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::gentypeparam>(ref);
	operands.name   = ref.ircode.stringrefs.ref(name);
	operands.atomid = (uint32_t) - 1;
	operands.setLVID(lvid);
	return offset;
}


inline uint32_t tparam(IRCodeRef ref, uint32_t lvid) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands  = blueprint::make<ir::isa::Blueprint::gentypeparam>(ref);
	operands.name   = 0;
	operands.atomid = (uint32_t) - 1;
	operands.setLVID(lvid);
	return offset;
}


} // namespace
} // namespace blueprint
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace emit {
namespace memory {
namespace {


inline uint32_t allocate(IRCodeRef ref, uint32_t lvid, uint32_t regsize) {
	auto& operands   = ref.ircode.emit<isa::Op::memalloc>();
	operands.lvid    = lvid;
	operands.regsize = regsize;
	return lvid;
}


inline void reallocate(IRCodeRef ref, uint32_t lvid, uint32_t oldsize, uint32_t newsize) {
	auto& operands   = ref.ircode.emit<isa::Op::memrealloc>();
	operands.lvid    = lvid;
	operands.oldsize = oldsize;
	operands.newsize = newsize;
}


inline void dispose(IRCodeRef ref, uint32_t lvid, uint32_t regsize) {
	auto& operands   = ref.ircode.emit<isa::Op::memfree>();
	operands.lvid    = lvid;
	operands.regsize = regsize;
}


inline void fill(IRCodeRef ref, uint32_t lvid, uint32_t regsize, uint32_t pattern) {
	auto& operands   = ref.ircode.emit<isa::Op::memfill>();
	operands.lvid    = lvid;
	operands.regsize = regsize;
	operands.pattern = pattern;
}


inline void copyNoOverlap(IRCodeRef ref, uint32_t lvid, uint32_t srclvid, uint32_t regsize) {
	auto& operands   = ref.ircode.emit<isa::Op::memcopy>();
	operands.lvid    = lvid;
	operands.srclvid = srclvid;
	operands.regsize = regsize;
}


inline void copy(IRCodeRef ref, uint32_t lvid, uint32_t srclvid, uint32_t regsize) {
	auto& operands   = ref.ircode.emit<isa::Op::memmove>();
	operands.lvid    = lvid;
	operands.srclvid = srclvid;
	operands.regsize = regsize;
}


inline void compare(IRCodeRef ref, uint32_t lvid, uint32_t srclvid, uint32_t regsize) {
	auto& operands   = ref.ircode.emit<isa::Op::memcmp>();
	operands.lvid    = lvid;
	operands.srclvid = srclvid;
	operands.regsize = regsize;
}


inline void cstrlen(IRCodeRef ref, uint32_t lvid, uint32_t bits, uint32_t ptr) {
	auto& operands = ref.ircode.emit<isa::Op::cstrlen>();
	operands.lvid  = lvid;
	operands.bits  = bits;
	operands.ptr   = ptr;
}


inline void loadu64(IRCodeRef ref, uint32_t lvid, uint32_t addr) {
	auto& opr   = ref.ircode.emit<isa::Op::load_u64>();
	opr.lvid    = lvid;
	opr.ptrlvid = addr;
}


inline void loadu32(IRCodeRef ref, uint32_t lvid, uint32_t addr) {
	auto& opr   = ref.ircode.emit<isa::Op::load_u32>();
	opr.lvid    = lvid;
	opr.ptrlvid = addr;
}


inline void loadu8(IRCodeRef ref, uint32_t lvid, uint32_t addr) {
	auto& opr   = ref.ircode.emit<isa::Op::load_u8>();
	opr.lvid    = lvid;
	opr.ptrlvid = addr;
}


inline void storeu64(IRCodeRef ref, uint32_t lvid, uint32_t addr) {
	auto& opr   = ref.ircode.emit<isa::Op::store_u64>();
	opr.lvid    = lvid;
	opr.ptrlvid = addr;
}


inline void storeu32(IRCodeRef ref, uint32_t lvid, uint32_t addr) {
	auto& opr   = ref.ircode.emit<isa::Op::store_u32>();
	opr.lvid    = lvid;
	opr.ptrlvid = addr;
}


inline void storeu8(IRCodeRef ref, uint32_t lvid, uint32_t addr) {
	auto& opr   = ref.ircode.emit<isa::Op::store_u8>();
	opr.lvid    = lvid;
	opr.ptrlvid = addr;
}


inline void hold(IRCodeRef ref, uint32_t lvid, uint32_t size) {
	auto& opr = ref.ircode.emit<isa::Op::memcheckhold>();
	opr.lvid = lvid;
	opr.size = size;
}


} // namespace
} // namespace memory
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace emit {
namespace pragma {
namespace {

inline auto& make(IRCodeRef& ref, ir::isa::Pragma value) {
	auto& operands = ref.ircode.emit<isa::Op::pragma>();
	operands.pragma = value;
	return operands;
}


//! Emit opcode that indicates the begining of a func body
inline void funcbody(IRCodeRef ref) {
	pragma::make(ref, ir::isa::Pragma::bodystart);
}


inline void synthetic(IRCodeRef ref, uint32_t lvid, bool onoff) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::synthetic);
	operands.value.synthetic.lvid  = lvid;
	operands.value.synthetic.onoff = static_cast<uint32_t>(onoff);
}


inline void suggest(IRCodeRef ref, bool onoff) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::suggest);
	operands.value.suggest = static_cast<uint32_t>(onoff);
}


//! Emit opcode to disable code generation
inline void codegen(IRCodeRef ref, bool enabled) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::codegen);
	operands.value.codegen = static_cast<uint32_t>(enabled);
}


inline void builtinAlias(IRCodeRef ref, const AnyString& name) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::builtinalias);
	operands.value.builtinalias.namesid = ref.ircode.stringrefs.ref(name);
}


inline void shortcircuit(IRCodeRef ref, bool evalvalue) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::shortcircuit);
	operands.value.shortcircuit = static_cast<uint32_t>(evalvalue);
}


inline void shortcircuitMetadata(IRCodeRef ref, uint32_t label) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::shortcircuitOpNopOffset);
	operands.value.shortcircuitMetadata.label = label;
}


inline void shortcircuitMutateToBool(IRCodeRef ref, uint32_t lvid, uint32_t source) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::shortcircuitMutateToBool);
	operands.value.shortcircuitMutate.lvid = lvid;
	operands.value.shortcircuitMutate.source = source;
}


inline void visibility(IRCodeRef ref, nyvisibility_t visibility) {
	auto& operands = pragma::make(ref, ir::isa::Pragma::visibility);
	operands.value.visibility = static_cast<uint32_t>(visibility);
}


inline uint32_t blueprintSize(IRCodeRef ref) {
	uint32_t offset = ref.ircode.opcodeCount();
	auto& operands = pragma::make(ref, ir::isa::Pragma::blueprintsize);
	operands.value.blueprintsize = 0;
	return offset;
}


} // namespace
} // namespace pragma
} // namespace emit
} // namespace ir
} // namespace ny
namespace ny {
namespace ir {
namespace emit {
namespace dbginfo {
namespace {


//! Emit a debug filename opcode
inline void filename(IRCodeRef ref, const AnyString& path) {
	ref.ircode.emit<isa::Op::debugfile>().filename = ref.ircode.stringrefs.ref(path);
}


inline void position(IRCodeRef ref, uint32_t line, uint32_t offset) {
	auto& operands  = ref.ircode.emit<isa::Op::debugpos>();
	operands.line   = line;
	operands.offset = offset;
}


} // namespace
} // namespace dbginfo
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace emit {
namespace {


struct ScopeLocker final {
	ScopeLocker(Sequence& ircode): ircode(ircode) {
		ir::emit::scopeBegin(ircode);
	}
	~ScopeLocker() {
		ir::emit::scopeEnd(ircode);
	}
	ScopeLocker(const ScopeLocker&) = delete;
	ScopeLocker(ScopeLocker&&) = default;
	Sequence& ircode;
};


struct CodegenLocker final {
	CodegenLocker(Sequence& ircode): ircode(ircode) {
		ir::emit::pragma::codegen(ircode, false);
	}
	~CodegenLocker() {
		ir::emit::pragma::codegen(ircode, true);
	}
	CodegenLocker(const CodegenLocker&) = delete;
	CodegenLocker(CodegenLocker&&) = default;
	Sequence& ircode;
};


} // namespace
} // namespace emit
} // namespace ir
} // namespace ny
