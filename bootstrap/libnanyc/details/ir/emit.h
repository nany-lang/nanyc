#pragma once
#include "sequence.h"
#include "details/ir/isa/data.h"


namespace ny
{
namespace ir
{
namespace emit
{
namespace
{

	struct SequenceRef final {
		SequenceRef(Sequence& sequence) :sequence(sequence) {}
		SequenceRef(Sequence* sequence) :sequence(*sequence) {
			assert(sequence != nullptr);
		}

		Sequence& sequence;
	};


	inline void opnot(SequenceRef ref, uint32_t lvid, uint32_t lhs)
	{
		auto& operands = ref.sequence.emit<ISA::Op::negation>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
	}


	inline void opfadd(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::fadd>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opfsub(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::fsub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opfdiv(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::fdiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opfmul(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::fmul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opadd(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::add>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opsub(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::sub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opdiv(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::div>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opmul(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::mul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opimul(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::imul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opidiv(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::idiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opeq(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::eq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opneq(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::neq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opflt(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::flt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opflte(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::flte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opfgt(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::fgt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opfgte(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::fgte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void oplt(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::lt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void oplte(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::lte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opilt(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::ilt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opilte(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::ilte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opgt(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::gt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opgte(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::gte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opigt(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::igt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opigte(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::igte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opand(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::opand>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opor(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::opor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opxor(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::opxor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void opmod(SequenceRef ref, uint32_t lvid, uint32_t lhs, uint32_t rhs) {
		auto& operands = ref.sequence.emit<ISA::Op::opmod>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void nop(SequenceRef ref) {
		ref.sequence.emit<ISA::Op::nop>();
	}


	//! Copy two register
	inline void copy(SequenceRef ref, uint32_t lvid, uint32_t source) {
		auto& operands = ref.sequence.emit<ISA::Op::store>();
		operands.lvid = lvid;
		operands.source = source;
	}


	inline void constantu64(SequenceRef ref, uint32_t lvid, uint64_t value) {
		auto& operands = ref.sequence.emit<ISA::Op::storeConstant>();
		operands.lvid = lvid;
		operands.value.u64 = value;
	}


	inline void constantf64(SequenceRef ref, uint32_t lvid, double value) {
		auto& operands = ref.sequence.emit<ISA::Op::storeConstant>();
		operands.lvid  = lvid;
		operands.value.f64 = value;
	}


	inline void constantbool(SequenceRef ref, uint32_t lvid, bool value) {
		constantu64(ref, lvid, value ? 1 : 0);
	}


	inline uint32_t constantText(SequenceRef ref, uint32_t lvid, const AnyString& text) {
		auto& operands = ref.sequence.emit<ISA::Op::storeText>();
		operands.lvid = lvid;
		operands.text = ref.sequence.stringrefs.ref(text);
		return operands.text;
	}


	//! Allocate a new variable on the stack and get the register
	inline uint32_t alloc(SequenceRef ref, uint32_t lvid, nytype_t type = nyt_any) {
		auto& operands  = ref.sequence.emit<ISA::Op::stackalloc>();
		operands.lvid   = lvid;
		operands.type   = static_cast<uint32_t>(type);
		operands.atomid = (uint32_t) -1;
		return lvid;
	}


	//! Allocate a new variable on the stack and assign a value to it and get the register
	inline uint32_t allocu64(SequenceRef ref, uint32_t lvid, nytype_t type, uint64_t value) {
		ir::emit::alloc(ref, lvid, type);
		ir::emit::constantu64(ref, lvid, value);
		return lvid;
	}
	

	//! Allocate a new variable on the stack and assign a value to it and get the register
	inline uint32_t allocf64(SequenceRef ref, uint32_t lvid, nytype_t type, double value) {
		ir::emit::alloc(ref, lvid, type);
		ir::emit::constantf64(ref, lvid, value);
		return lvid;
	}


	//! Allocate a new variable on the stack and assign a text to it and get the register
	inline uint32_t alloctext(SequenceRef ref, uint32_t lvid, const AnyString& text) {
		ir::emit::alloc(ref, lvid, nyt_ptr);
		ir::emit::constantText(ref, lvid, text);
		return lvid;
	}


	inline uint32_t increaseStacksize(SequenceRef ref, uint32_t size = 0) {
		uint32_t offset = ref.sequence.opcodeCount();
		ref.sequence.emit<ISA::Op::stacksize>().add = size;
		return offset;
	}


	inline void cassert(SequenceRef ref, uint32_t lvid) {
		ref.sequence.emit<ISA::Op::opassert>().lvid = lvid;
	}


	inline uint32_t objectAlloc(SequenceRef ref, uint32_t lvid, uint32_t atomid) {
		auto& operands = ref.sequence.emit<ISA::Op::allocate>();
		operands.lvid = lvid;
		operands.atomid = atomid;
		return lvid;
	}


	inline void ref(SequenceRef ref, uint32_t lvid) {
		ref.sequence.emit<ISA::Op::ref>().lvid = lvid;
	}


	inline void unref(SequenceRef ref, uint32_t lvid, uint32_t atomid, uint32_t instanceid) {
		auto& operands = ref.sequence.emit<ISA::Op::unref>();
		operands.lvid = lvid;
		operands.atomid = atomid;
		operands.instanceid = instanceid;
	}


	inline void scopeBegin(SequenceRef ref) {
		ref.sequence.emit<ISA::Op::scope>();
	}


	inline void scopeEnd(SequenceRef ref) {
		ref.sequence.emit<ISA::Op::end>();
	}


	inline uint32_t label(SequenceRef ref, uint32_t labelid) {
		ref.sequence.emit<ISA::Op::label>().label = labelid;
		return labelid;
	}


	//! Unconditional jump
	inline void jmp(SequenceRef ref, uint32_t label) {
		ref.sequence.emit<ISA::Op::jmp>().label = label;
	}


	//! jump if zero
	inline void jz(SequenceRef ref, uint32_t lvid, uint32_t result, uint32_t label) {
		auto& opc  = ref.sequence.emit<ISA::Op::jz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}


	//! jump if not zero
	inline void jnz(SequenceRef ref, uint32_t lvid, uint32_t result, uint32_t label) {
		auto& opc  = ref.sequence.emit<ISA::Op::jnz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}


	inline void identify(SequenceRef ref, uint32_t lvid, const AnyString& name, uint32_t self) {
		auto& sequence = ref.sequence;
		auto& operands = sequence.emit<ISA::Op::identify>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.text  = sequence.stringrefs.ref(name);
	}


	inline void namealias(SequenceRef ref, uint32_t lvid, const AnyString& name) {
		auto& sequence = ref.sequence;
		auto& operands = sequence.emit<ISA::Op::namealias>();
		operands.lvid  = lvid;
		operands.name  = sequence.stringrefs.ref(name);
	}


	inline void push(SequenceRef ref, uint32_t lvid) {
		auto& operands = ref.sequence.emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = 0;
	}


	inline void push(SequenceRef ref, uint32_t lvid, const AnyString& name) {
		auto& sequence = ref.sequence;
		auto& operands = sequence.emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = (not name.empty()) ? sequence.stringrefs.ref(name) : 0;
	}


	inline void tpush(SequenceRef ref, uint32_t lvid) {
		auto& operands = ref.sequence.emit<ISA::Op::tpush>();
		operands.lvid  = lvid;
		operands.name  = 0;
	}


	inline void tpush(SequenceRef ref, uint32_t lvid, const AnyString& name) {
		auto& operands = ref.sequence.emit<ISA::Op::tpush>();
		operands.lvid  = lvid;
		operands.name  = (not name.empty()) ? ref.sequence.stringrefs.ref(name) : 0;
	}


	inline void call(SequenceRef ref, uint32_t lvid, uint32_t ptr2func) {
		auto& operands = ref.sequence.emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = ptr2func;
		operands.instanceid = (uint32_t) -1;
	}


	inline void call(SequenceRef ref, uint32_t lvid, uint32_t atomid, uint32_t instanceid) {
		assert(instanceid != (uint32_t) -1);
		auto& operands = ref.sequence.emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = atomid;
		operands.instanceid = instanceid;
	}


	inline void intrinsic(SequenceRef ref, uint32_t lvid, const AnyString& name, uint32_t id = (uint32_t) -1) {
		auto& operands     = ref.sequence.emit<ISA::Op::intrinsic>();
		operands.lvid      = lvid;
		operands.intrinsic = ref.sequence.stringrefs.ref(name);
		operands.iid       = id;
	}


	//! Return with no value
	inline void ret(SequenceRef ref) {
		auto& operands   = ref.sequence.emit<ISA::Op::ret>();
		operands.lvid    = 0;
		operands.tmplvid = 0;
	}


	//! Return from value
	inline void ret(SequenceRef ref, uint32_t lvid, uint32_t tmplvid) {
		auto& operands   = ref.sequence.emit<ISA::Op::ret>();
		operands.lvid    = lvid;
		operands.tmplvid = tmplvid;
	}


	inline void fieldget(SequenceRef ref, uint32_t lvid, uint32_t self, uint32_t fieldindex) {
		assert(lvid != 0 and self != 0);
		auto& operands = ref.sequence.emit<ISA::Op::fieldget>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = fieldindex;
	}


	inline void fieldset(SequenceRef ref, uint32_t lvid, uint32_t self, uint32_t varid) {
		assert(lvid != 0 and self != 0);
		auto& operands = ref.sequence.emit<ISA::Op::fieldset>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = varid;
	}


	template<class T> struct TraceWriter final {
		static void emit(SequenceRef ref, const T& value) {
			auto& sequence = ref.sequence;
			sequence.emit<ISA::Op::comment>().text = sequence.stringrefs.ref(value());
		}
	};


	template<uint32_t N>
	struct TraceWriter<char[N]> final {
		static void emit(SequenceRef ref, const char* value) {
			auto& sequence = ref.sequence;
			sequence.emit<ISA::Op::comment>().text = sequence.stringrefs.ref(AnyString{value, N});
		}
	};


	//! Insert a comment in the IR code
	template<class T>
	inline void trace(SequenceRef ref, const T& value) {
		//! ir::emit::trace(out, [](){return "some comments here";});
		if (yuni::debugmode)
			TraceWriter<T>::emit(std::move(ref), value);
	}


	//! Insert a comment in the IR code
	template<class T>
	inline void trace(SequenceRef ref, bool condition, const T& value) {
		//! ir::emit::trace(out, [](){return "some comments here";});
		if (yuni::debugmode and condition)
			TraceWriter<T>::emit(std::move(ref), value);
	}


	//! Insert empty comment line in the IR code
	inline void trace(SequenceRef ref) {
		if (yuni::debugmode)
			ref.sequence.emit<ISA::Op::comment>().text = 0;
	}


} // namespace
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny
{
namespace ir
{
namespace emit
{
namespace type
{
namespace
{

	inline void isself(SequenceRef ref, uint32_t lvid) {
		ref.sequence.emit<ISA::Op::self>().self = lvid;
	}


	inline void isobject(SequenceRef ref, uint32_t lvid) {
		ref.sequence.emit<ISA::Op::typeisobject>().lvid = lvid;
	}


	inline void common(SequenceRef ref, uint32_t lvid, uint32_t previous) {
		auto& operands = ref.sequence.emit<ISA::Op::commontype>();
		operands.lvid = lvid;
		operands.previous = previous;
	}


	inline void objectSizeof(SequenceRef ref, uint32_t lvid, uint32_t type) {
		auto& operands = ref.sequence.emit<ISA::Op::classdefsizeof>();
		operands.lvid  = lvid;
		operands.type  = type;
	}


	inline void qualifierRef(SequenceRef ref, uint32_t lvid, bool flag) {
		auto& operands = ref.sequence.emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = ir::ISA::TypeQualifier::ref;
	}


	inline void qualifierConst(SequenceRef ref, uint32_t lvid, bool flag) {
		auto& operands = ref.sequence.emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = ir::ISA::TypeQualifier::constant;
	}


	inline void ensureResolved(SequenceRef ref, uint32_t lvid) {
		ref.sequence.emit<ISA::Op::ensureresolved>().lvid = lvid;
	}


} // namespace
} // namespace type
} // namespace emit
} // namespace ir
} // namespace ny



namespace ny
{
namespace ir
{
namespace emit
{
namespace memory
{
namespace
{


	inline uint32_t allocate(SequenceRef ref, uint32_t lvid, uint32_t regsize) {
		auto& operands   = ref.sequence.emit<ISA::Op::memalloc>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
		return lvid;
	}


	inline void reallocate(SequenceRef ref, uint32_t lvid, uint32_t oldsize, uint32_t newsize) {
		auto& operands   = ref.sequence.emit<ISA::Op::memrealloc>();
		operands.lvid    = lvid;
		operands.oldsize = oldsize;
		operands.newsize = newsize;
	}


	inline void dispose(SequenceRef ref, uint32_t lvid, uint32_t regsize) {
		auto& operands   = ref.sequence.emit<ISA::Op::memfree>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
	}


	inline void fill(SequenceRef ref, uint32_t lvid, uint32_t regsize, uint32_t pattern) {
		auto& operands   = ref.sequence.emit<ISA::Op::memfill>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
		operands.pattern = pattern;
	}


	inline void copyNoOverlap(SequenceRef ref, uint32_t lvid, uint32_t srclvid, uint32_t regsize) {
		auto& operands   = ref.sequence.emit<ISA::Op::memcopy>();
		operands.lvid    = lvid;
		operands.srclvid = srclvid;
		operands.regsize = regsize;
	}


	inline void copy(SequenceRef ref, uint32_t lvid, uint32_t srclvid, uint32_t regsize) {
		auto& operands   = ref.sequence.emit<ISA::Op::memmove>();
		operands.lvid    = lvid;
		operands.srclvid = srclvid;
		operands.regsize = regsize;
	}


	inline void compare(SequenceRef ref, uint32_t lvid, uint32_t srclvid, uint32_t regsize) {
		auto& operands   = ref.sequence.emit<ISA::Op::memcmp>();
		operands.lvid    = lvid;
		operands.srclvid = srclvid;
		operands.regsize = regsize;
	}


	inline void cstrlen(SequenceRef ref, uint32_t lvid, uint32_t bits, uint32_t ptr) {
		auto& operands = ref.sequence.emit<ISA::Op::cstrlen>();
		operands.lvid  = lvid;
		operands.bits  = bits;
		operands.ptr   = ptr;
	}


	inline void loadu64(SequenceRef ref, uint32_t lvid, uint32_t addr) {
		auto& opr   = ref.sequence.emit<ISA::Op::load_u64>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}


	inline void loadu32(SequenceRef ref, uint32_t lvid, uint32_t addr) {
		auto& opr   = ref.sequence.emit<ISA::Op::load_u32>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}


	inline void loadu8(SequenceRef ref, uint32_t lvid, uint32_t addr) {
		auto& opr   = ref.sequence.emit<ISA::Op::load_u8>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}

	
	inline void storeu64(SequenceRef ref, uint32_t lvid, uint32_t addr) {
		auto& opr   = ref.sequence.emit<ISA::Op::store_u64>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}


	inline void storeu32(SequenceRef ref, uint32_t lvid, uint32_t addr) {
		auto& opr   = ref.sequence.emit<ISA::Op::store_u32>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}


	inline void storeu8(SequenceRef ref, uint32_t lvid, uint32_t addr) {
		auto& opr   = ref.sequence.emit<ISA::Op::store_u8>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}


	inline void hold(SequenceRef ref, uint32_t lvid, uint32_t size) {
		auto& opr = ref.sequence.emit<ISA::Op::memcheckhold>();
		opr.lvid = lvid;
		opr.size = size;
	}


} // namespace
} // namespace memory
} // namespace emit
} // namespace ir
} // namespace ny


namespace ny
{
namespace ir
{
namespace emit
{
namespace pragma
{
namespace
{

	inline auto& make(SequenceRef& ref, ir::ISA::Pragma value) {
		auto& operands = ref.sequence.emit<ISA::Op::pragma>();
		operands.pragma = value;
		return operands;
	}


	//! Emit opcode that indicates the begining of a func body
	inline void funcbody(SequenceRef ref) {
		pragma::make(ref, ir::ISA::Pragma::bodystart);
	}


	inline void synthetic(SequenceRef ref, uint32_t lvid, bool onoff) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::synthetic);
		operands.value.synthetic.lvid  = lvid;
		operands.value.synthetic.onoff = static_cast<uint32_t>(onoff);
	}


	inline void suggest(SequenceRef ref, bool onoff) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::suggest);
		operands.value.suggest = static_cast<uint32_t>(onoff);
	}


	//! Emit opcode to disable code generation
	inline void codegen(SequenceRef ref, bool enabled) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::codegen);
		operands.value.codegen = static_cast<uint32_t>(enabled);
	}


	inline void builtinAlias(SequenceRef ref, const AnyString& name) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::builtinalias);
		operands.value.builtinalias.namesid = ref.sequence.stringrefs.ref(name);
	}


	inline void shortcircuit(SequenceRef ref, bool evalvalue) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::shortcircuit);
		operands.value.shortcircuit = static_cast<uint32_t>(evalvalue);
	}


	inline void shortcircuitMetadata(SequenceRef ref, uint32_t label) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::shortcircuitOpNopOffset);
		operands.value.shortcircuitMetadata.label = label;
	}


	inline void shortcircuitMutateToBool(SequenceRef ref, uint32_t lvid, uint32_t source) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::shortcircuitMutateToBool);
		operands.value.shortcircuitMutate.lvid = lvid;
		operands.value.shortcircuitMutate.source = source;
	}


	inline void visibility(SequenceRef ref, nyvisibility_t visibility) {
		auto& operands = pragma::make(ref, ir::ISA::Pragma::visibility);
		operands.value.visibility = static_cast<uint32_t>(visibility);
	}


	inline uint32_t blueprintSize(SequenceRef ref) {
		uint32_t offset = ref.sequence.opcodeCount();
		auto& operands = pragma::make(ref, ir::ISA::Pragma::blueprintsize);
		operands.value.blueprintsize = 0;
		return offset;
	}


} // namespace
} // namespace pragma
} // namespace emit
} // namespace ir
} // namespace ny
namespace ny
{
namespace ir
{
namespace emit
{
namespace dbginfo
{
namespace
{


	//! Emit a debug filename opcode
	inline void filename(SequenceRef ref, const AnyString& path) {
		auto& sequence = ref.sequence;
		sequence.emit<ISA::Op::debugfile>().filename = sequence.stringrefs.ref(path);
	}


	inline void position(SequenceRef ref, uint32_t line, uint32_t offset) {
		auto& operands  = ref.sequence.emit<ISA::Op::debugpos>();
		operands.line   = line;
		operands.offset = offset;
	}


} // namespace
} // namespace dbginfo
} // namespace emit
} // namespace ir
} // namespace ny
