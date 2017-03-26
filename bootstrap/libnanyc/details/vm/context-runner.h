#pragma once
#include "details/vm/context.h"
#include "details/vm/types.h"
#include "stack.h"
#include "stacktrace.h"
#include "memchecker.h"
#include "dyncall/dyncall.h"
#include "details/ir/isa/data.h"
#include <iostream>
#include <csetjmp>
#include <limits>


//! Print opcodes executed by the vm
#define ny_vm_PRINT_OPCODES 0


#define VM_CHECK_POINTER(P,LVID)  if (unlikely(not memchecker.has((P)))) \
	{ \
		emitUnknownPointer((P)); /*assert(false and "invalid pointer");*/ \
	}


#if ny_vm_PRINT_OPCODES != 0
#define vm_PRINT_OPCODE(O)  do { std::cout << "== ny:vm +" \
		<< ircode.get().offsetOf(opr) << "  == "  \
		<< ny::ir::isa::print(ircode.get(), opr, &map) << '\n';} while (0)
#else
#define vm_PRINT_OPCODE(O)
#endif

#define ASSERT_LVID(L)  assert((L) > 0 and (L) < registerCount)




namespace ny {
namespace vm {


struct ContextRunner final {
	constexpr static const int patternAlloc = 0xCD;
	constexpr static const int patternFree = 0xCD;

	struct Exception: public std::exception {};
	struct Abort final: public Exception {};
	struct DyncallError final: public Exception {};

	//! Registers for the current stack frame
	DataRegister* registers = nullptr;
	//! Return value
	uint64_t retRegister = 0;
	//! Number of pushed parameters
	uint32_t funcparamCount = 0; // parameters are 2-based
	//! all pushed parameters
	DataRegister funcparams[config::maxPushedParameters];

	nyallocator_t& allocator;
	DCCallVM* dyncall = nullptr;
	nyvm_t cfvm;
	nyprogram_cf_t cf;
	Context& context;
	Stack stack;
	Stacktrace<true> stacktrace;
	MemChecker<true> memchecker;
	//! upper label id encountered so far
	uint32_t upperLabelID = 0;
	const AtomMap& map;
	std::reference_wrapper<const ir::Sequence> ircode;
	const ny::intrinsic::Catalog& userDefinedIntrinsics;
	const ir::Instruction** cursor = nullptr;
	#ifndef NDEBUG
	//! Total number of registers in the current frame
	uint32_t registerCount = 0;
	#endif

public:
	ContextRunner(Context&, const ir::Sequence& callee);
	~ContextRunner();
	void initialize();

	[[noreturn]] void abortMission();
	[[noreturn]] void emitBadAlloc();
	[[noreturn]] void emitPointerSizeMismatch(void* object, size_t size);
	[[noreturn]] void emitAssert();
	[[noreturn]] void emitUnexpectedOpcode(ir::isa::Op);
	[[noreturn]] void emitInvalidIntrinsicParamType();
	[[noreturn]] void emitInvalidReturnType();
	[[noreturn]] void emitDividedByZero();
	[[noreturn]] void emitUnknownPointer(void* p);
	[[noreturn]] void emitLabelError(uint32_t label);


	template<class T> T* allocateraw(size_t size) {
		return (T*) allocator.allocate(&allocator, size);
	}


	void deallocate(void* object, size_t size) {
		assert(object != nullptr);
		allocator.deallocate(&allocator, object, size);
	}


	void destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid);


	void gotoLabel(uint32_t label) {
		bool jmpsuccess = (label > upperLabelID)
			? ircode.get().jumpToLabelForward(*cursor, label)
			: ircode.get().jumpToLabelBackward(*cursor, label);
		if (unlikely(not jmpsuccess))
			emitLabelError(label);
		upperLabelID = label; // the labels are strictly ordered
	}


	// accept those opcode for debugging purposes
	void visit(const ir::isa::Operand<ir::isa::Op::comment>&) {}
	void visit(const ir::isa::Operand<ir::isa::Op::scope>&) {}
	void visit(const ir::isa::Operand<ir::isa::Op::end>&) {}
	void visit(const ir::isa::Operand<ir::isa::Op::nop>&) {}


	void visit(const ir::isa::Operand<ir::isa::Op::negation>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		registers[opr.lvid].u64 = not registers[opr.lhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::intrinsic>& opr);


	void visit(const ir::isa::Operand<ir::isa::Op::fadd>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].f64 = registers[opr.lhs].f64 + registers[opr.rhs].f64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fsub>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].f64 = registers[opr.lhs].f64 - registers[opr.rhs].f64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fmul>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].f64 = registers[opr.lhs].f64 * registers[opr.rhs].f64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fdiv>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		auto r = registers[opr.rhs].f64;
		if (unlikely((uint64_t)r == 0))
			emitDividedByZero();
		registers[opr.lvid].f64 = registers[opr.lhs].f64 / r;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::add>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 + registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::sub>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 - registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::mul>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 * registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::div>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		auto r = registers[opr.rhs].u64;
		if (unlikely(r == 0))
			emitDividedByZero();
		registers[opr.lvid].u64 = registers[opr.lhs].u64 / r;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::imul>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		auto lhs = static_cast<int64_t>(registers[opr.lhs].u64);
		auto rhs = static_cast<int64_t>(registers[opr.rhs].u64);
		registers[opr.lvid].u64 = static_cast<uint64_t>(lhs * rhs);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::idiv>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		auto r = static_cast<int64_t>(registers[opr.rhs].u64);
		if (unlikely(r == 0))
			emitDividedByZero();
		registers[opr.lvid].u64 = static_cast<uint64_t>(static_cast<int64_t>(registers[opr.lhs].u64) / r);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::eq>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 == registers[opr.rhs].u64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::neq>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 != registers[opr.rhs].u64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::lt>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 < registers[opr.rhs].u64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::lte>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 <= registers[opr.rhs].u64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::ilt>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 < registers[opr.rhs].i64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::ilte>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 <= registers[opr.rhs].i64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::gt>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 > registers[opr.rhs].u64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::gte>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 >= registers[opr.rhs].u64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::igt>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 > registers[opr.rhs].i64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::igte>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 >= registers[opr.rhs].i64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::flt>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 < registers[opr.rhs].f64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::flte>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 <= registers[opr.rhs].f64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fgt>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 > registers[opr.rhs].f64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fgte>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 >= registers[opr.rhs].f64) ? 1 : 0;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::opand>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 & registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::opor>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 | registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::opxor>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 ^ registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::opmod>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lhs);
		ASSERT_LVID(opr.rhs);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 % registers[opr.rhs].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::push>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.lvid);
		funcparams[funcparamCount++].u64 = registers[opr.lvid].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::ret>& opr) {
		vm_PRINT_OPCODE(opr);
		assert(opr.lvid == 0 or opr.lvid < registerCount);
		retRegister = registers[opr.lvid].u64;
		ircode.get().invalidateCursor(*cursor);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::store>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.source);
		registers[opr.lvid] = registers[opr.source];
	}


	void visit(const ir::isa::Operand<ir::isa::Op::storeText>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		auto cstr = ircode.get().stringrefs[opr.text].c_str();
		registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(cstr);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::storeConstant>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		registers[opr.lvid].u64 = opr.value.u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::classdefsizeof>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		assert(map.findAtom(opr.type) != nullptr);
		registers[opr.lvid].u64 = map.findAtom(opr.type)->runtimeSizeof();
	}


	void visit(const ir::isa::Operand<ir::isa::Op::call>& opr) {
		ASSERT_LVID(opr.lvid);
		vm_PRINT_OPCODE(opr);
		call(opr.lvid, opr.ptr2func, opr.instanceid);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fieldset>& opr) {
		assert(opr.self < registerCount);
		ASSERT_LVID(opr.lvid);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.self].u64);
		VM_CHECK_POINTER(object, opr);
		object[1 + opr.var] = registers[opr.lvid].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::fieldget>& opr) {
		ASSERT_LVID(opr.self);
		ASSERT_LVID(opr.lvid);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.self].u64);
		VM_CHECK_POINTER(object, opr);
		registers[opr.lvid].u64 = object[1 + opr.var];
	}


	void visit(const ir::isa::Operand<ir::isa::Op::label>& opr) {
		if (opr.label > upperLabelID)
			upperLabelID = opr.label;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::jmp>& opr) {
		gotoLabel(opr.label);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::jnz>& opr) {
		if (registers[opr.lvid].u64 != 0) {
			registers[opr.result].u64 = 1;
			gotoLabel(opr.label);
		}
	}

	void visit(const ir::isa::Operand<ir::isa::Op::jz>& opr) {
		if (registers[opr.lvid].u64 == 0) {
			registers[opr.result].u64 = 0;
			gotoLabel(opr.label);
		}
	}


	void visit(const ir::isa::Operand<ir::isa::Op::ref>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		VM_CHECK_POINTER(object, opr);
		++(object[0]); // +ref
	}


	void visit(const ir::isa::Operand<ir::isa::Op::unref>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		VM_CHECK_POINTER(object, opr);
		if (0 == --(object[0])) // -unref
			destroy(object, opr.atomid, opr.instanceid);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::dispose>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		VM_CHECK_POINTER(object, opr);
		destroy(object, opr.atomid, opr.instanceid);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::stackalloc>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		(void) opr;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::memalloc>& opr);
	void visit(const ir::isa::Operand<ir::isa::Op::memrealloc>& opr);
	void visit(const ir::isa::Operand<ir::isa::Op::memfree>& opr);


	void visit(const ir::isa::Operand<ir::isa::Op::memcheckhold>& opr) {
		uint64_t* ptr = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t size = registers[opr.size].u64 + config::extraObjectSize;
		memchecker.hold(ptr, size, opr.lvid);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::memfill>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.regsize);
		ASSERT_LVID(opr.pattern);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		uint8_t pattern = static_cast<uint8_t>(registers[opr.pattern].u64);
		memset(object, pattern, size);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::memcopy>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.srclvid);
		ASSERT_LVID(opr.regsize);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t* src = reinterpret_cast<uint64_t*>(registers[opr.srclvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		memcpy(object, src, size);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::memmove>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.srclvid);
		ASSERT_LVID(opr.regsize);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t* src = reinterpret_cast<uint64_t*>(registers[opr.srclvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		memmove(object, src, size);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::memcmp>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.srclvid);
		ASSERT_LVID(opr.regsize);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t* src = reinterpret_cast<uint64_t*>(registers[opr.srclvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		int cmp = memcmp(object, src, size);
		registers[opr.regsize].u64 = (cmp == 0) ? 0 : ((cmp < 0) ? 2 : 1);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::cstrlen>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptr);
		auto* cstring = reinterpret_cast<const char*>(registers[opr.ptr].u64);
		size_t clen = cstring ? strlen(cstring) : 0u;
		registers[opr.lvid].u64 = clen;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::load_u64>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptrlvid);
		registers[opr.lvid].u64 = *(reinterpret_cast<uint64_t*>(registers[opr.ptrlvid].u64));
	}


	void visit(const ir::isa::Operand<ir::isa::Op::load_u32>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptrlvid);
		registers[opr.lvid].u64 = *(reinterpret_cast<uint32_t*>(registers[opr.ptrlvid].u64));
	}


	void visit(const ir::isa::Operand<ir::isa::Op::load_u8>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptrlvid);
		registers[opr.lvid].u64 = *(reinterpret_cast<uint8_t*>(registers[opr.ptrlvid].u64));
	}


	void visit(const ir::isa::Operand<ir::isa::Op::store_u64>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptrlvid);
		*(reinterpret_cast<uint64_t*>(registers[opr.ptrlvid].u64)) = registers[opr.lvid].u64;
	}


	void visit(const ir::isa::Operand<ir::isa::Op::store_u32>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptrlvid);
		*(reinterpret_cast<uint32_t*>(registers[opr.ptrlvid].u64)) = static_cast<uint32_t>(registers[opr.lvid].u64);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::store_u8>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		ASSERT_LVID(opr.ptrlvid);
		*(reinterpret_cast<uint8_t*>(registers[opr.ptrlvid].u64)) = static_cast<uint8_t>(registers[opr.lvid].u64);
	}


	void visit(const ir::isa::Operand<ir::isa::Op::opassert>& opr) {
		vm_PRINT_OPCODE(opr);
		ASSERT_LVID(opr.lvid);
		if (unlikely(registers[opr.lvid].u64 == 0))
			return emitAssert();
	}

	template<ir::isa::Op O> void visit(const ir::isa::Operand<O>& opr) {
		vm_PRINT_OPCODE(opr);
		(void) opr;
		return emitUnexpectedOpcode(O);
	}


	uint64_t invoke(const ir::Sequence& callee);

	void call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid);

}; // struct ContextRunner


} // namespace vm
} // namespace ny
