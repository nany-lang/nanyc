#include "dyncall/dyncall.h"
#include "details/vm/thread.h"
#include "details/vm/allocator.h"
#include "details/atom/atom.h"
#include "details/vm/stack.h"
#include "details/vm/stacktrace.h"
#include <iostream>


#if defined(__GNUC__)
#  define NYVM_NOINLINE  __attribute__((noinline))
#elif defined(_MSC_VER) && _MSC_VER>=1310
#  define NYVM_NOINLINE  __declspec(noinline)
#else
#  define NYVM_NOINLINE
#endif

namespace ny {
namespace vm {

namespace {

constexpr bool printOpcodes = false;

struct InvalidLabel final {
	InvalidLabel(uint32_t atomid, uint32_t label): atomid(atomid), label(label) {}
	uint32_t atomid;
	uint32_t label;
};

struct DivideByZero final {
};

struct Assert final {
};

struct UnexpectedOpcode final {
	UnexpectedOpcode(const AnyString& name): name(name) {}
	AnyString name;
};

struct InvalidDtor final {
	InvalidDtor(const Atom&) {}
};

struct ICE final {
	ICE(uint8_t line, const char* msg): line(line), msg(msg) {}
	const char* file = __FILE__;
	uint32_t line;
	const char* msg;
};

struct Executor final {
	using Allocator = ny::vm::memory::Allocator<ny::vm::memory::TrackPointer>;

	Register* registers = nullptr; // current registers
	Register retval;
	Stack stack;
	Stacktrace<true> stacktrace;
	uint32_t paramCount = 0;
	Register parameters[config::maxPushedParameters];
	uint32_t upperLabelID = 0;
	std::reference_wrapper<const ir::Sequence> ircode; // current ir sequence
	DCCallVM* dyncall = nullptr;
	Allocator allocator;
	bool unwindRaisedError = false;
	void* raisedError = nullptr;
	uint32_t raisedErrorAtomid = 0;
	const AtomMap& map;
	const ny::intrinsic::Catalog& intrinsics;
	ny::vm::Thread& thread;
	const ir::Instruction** cursor = nullptr;

	struct {
		#ifndef NDEBUG
		void registerCount(uint32_t x) { m_registerCount = x; }
		uint32_t registerCount() const { return m_registerCount; }
		uint32_t m_registerCount = 0;
		#else
		void registerCount(uint32_t) {}
		constexpr uint32_t registerCount() const { return 0; }
		#endif
		uint32_t calldepth = 0;
	}
	dbg;

	Executor(ny::vm::Thread& thread, const ny::ir::Sequence& sequence)
		: ircode(std::cref(sequence))
		, dyncall(dcNewCallVM(4096))
		, map(thread.machine.program.compdb->cdeftable.atoms)
		, intrinsics(thread.machine.program.compdb->intrinsics)
		, thread(thread) {
		if (unlikely(!dyncall))
			throw "failed to call dcNewCallVM";
		dcMode(dyncall, DC_CALL_C_DEFAULT);
	}

	~Executor() {
		if (dyncall)
			dcFree(dyncall);
	}

	NYVM_NOINLINE void destroy(uint64_t* object, uint32_t dtorid);
	NYVM_NOINLINE void call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid);
	inline uint64_t entrypoint(uint32_t atomfunc, uint32_t instanceid);

	void validateLvids(uint32_t lvid) const {
		assert(lvid > 0 and lvid < dbg.registerCount() and "invalid lvid");
	}

	void validateLvids(uint32_t lvid, uint32_t list...) const {
		validateLvids(lvid);
		validateLvids(list);
	}

	template<class O> void validateLvids(const O& opr) const {
		if (yuni::debugmode) {
			(const_cast<O&>(opr)).eachLVID([&](uint32_t list...) {
				validateLvids(list);
			});
		}
	}

	template<class O> void printOpcode(const O& opr) const {
		if (printOpcodes) {
			std::cout << "== nanyc:vm d:" << dbg.calldepth << " +";
			std::cout << ircode.get().offsetOf(opr) << "  == ";
			std::cout << ny::ir::isa::print(ircode.get(), opr, &map) << '\n';
		}
	}

	void returnFromCurrentFunc(uint64_t val = 0) {
		retval.u64 = val;
		ircode.get().invalidateCursor(*cursor);
	}

	void gotoLabel(uint32_t label) {
		auto& sequence = ircode.get();
		bool jmpsuccess = (label > upperLabelID)
			? sequence.jumpToLabelForward(*cursor, label)
			: sequence.jumpToLabelBackward(*cursor, label);
		if (unlikely(not jmpsuccess))
			throw InvalidLabel(allocator.tracker.atomid(), label);
		upperLabelID = label; // the labels are strictly ordered
	}

	void visit(const ir::isa::Operand<ir::isa::Op::negation>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = not registers[opr.lhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::intrinsic>& opr) {
		printOpcode(opr);
		dcReset(dyncall);
		dcArgPointer(dyncall, &thread.capi);
		auto& intrinsic = intrinsics[opr.iid];
		for (uint32_t i = 0; i != paramCount; ++i) {
			auto r = parameters[i];
			switch (intrinsic.params[i]) {
				case CType::t_u64:
						dcArgLongLong(dyncall, static_cast<DClonglong>(r.u64));
					break;
				case CType::t_i64:
					dcArgLongLong(dyncall, static_cast<DClonglong>(r.i64));
					break;
				case CType::t_u32:
					dcArgInt(dyncall, static_cast<DCint>(r.u64));
					break;
				case CType::t_i32:
					dcArgInt(dyncall, static_cast<DCint>(r.i64));
					break;
				case CType::t_ptr:
					dcArgPointer(dyncall, reinterpret_cast<DCpointer>(r.u64));
					break;
				case CType::t_u16:
					dcArgShort(dyncall, static_cast<DCshort>(r.u64));
					break;
				case CType::t_i16:
					dcArgShort(dyncall, static_cast<DCshort>(r.i64));
					break;
				case CType::t_u8:
					dcArgChar(dyncall, static_cast<DCchar>(r.u64));
					break;
				case CType::t_i8:
					dcArgChar(dyncall, static_cast<DCchar>(r.i64));
					break;
				case CType::t_f32:
					dcArgFloat(dyncall, static_cast<DCfloat>(r.f64));
					break;
				case CType::t_f64:
					dcArgDouble(dyncall, static_cast<DCdouble>(r.f64));
					break;
				case CType::t_bool:
					dcArgBool(dyncall, static_cast<DCbool>(r.u64));
					break;
				case CType::t_void:
				case CType::t_any:
					throw ICE(__LINE__, "invalid intrinsic parameter type");
			}
		}
		paramCount = 0;
		switch (intrinsic.rettype) {
			case CType::t_u64:
				registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallLongLong(dyncall, intrinsic.callback));
				break;
			case CType::t_i64:
				registers[opr.lvid].i64 = static_cast<int64_t>(dcCallLongLong(dyncall, intrinsic.callback));
				break;
			case CType::t_u32:
				registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallInt(dyncall, intrinsic.callback));
				break;
			case CType::t_i32:
				registers[opr.lvid].i64 = static_cast<int64_t>(dcCallInt(dyncall, intrinsic.callback));
				break;
			case CType::t_ptr:
				registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(dcCallPointer(dyncall, intrinsic.callback));
				break;
			case CType::t_u16:
				registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallShort(dyncall, intrinsic.callback));
				break;
			case CType::t_i16:
				registers[opr.lvid].i64 = static_cast<int64_t>(dcCallShort(dyncall, intrinsic.callback));
				break;
			case CType::t_u8:
				registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallChar(dyncall, intrinsic.callback));
				break;
			case CType::t_i8:
				registers[opr.lvid].i64 = static_cast<int64_t>(dcCallChar(dyncall, intrinsic.callback));
				break;
			case CType::t_f32:
				registers[opr.lvid].f64 = static_cast<float>(dcCallFloat(dyncall, intrinsic.callback));
				break;
			case CType::t_f64:
				registers[opr.lvid].f64 = static_cast<double>(dcCallDouble(dyncall, intrinsic.callback));
				break;
			case CType::t_bool:
				registers[opr.lvid].u64 = (dcCallBool(dyncall, intrinsic.callback) ? 1 : 0);
				break;
			case CType::t_void:
				dcCallVoid(dyncall, intrinsic.callback);
				break;
			case CType::t_any:
				throw ICE(__LINE__, "invalid intrinsic return type");
		}
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fadd>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].f64 = registers[opr.lhs].f64 + registers[opr.rhs].f64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fsub>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].f64 = registers[opr.lhs].f64 - registers[opr.rhs].f64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fmul>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].f64 = registers[opr.lhs].f64 * registers[opr.rhs].f64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fdiv>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		auto r = registers[opr.rhs].f64;
		if (unlikely((uint64_t)r == 0))
			throw DivideByZero();
		registers[opr.lvid].f64 = registers[opr.lhs].f64 / r;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::add>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 + registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::sub>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 - registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::mul>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 * registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::div>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		auto r = registers[opr.rhs].u64;
		if (unlikely(r == 0))
			throw DivideByZero();
		registers[opr.lvid].u64 = registers[opr.lhs].u64 / r;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::imul>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		auto lhs = static_cast<int64_t>(registers[opr.lhs].u64);
		auto rhs = static_cast<int64_t>(registers[opr.rhs].u64);
		registers[opr.lvid].u64 = static_cast<uint64_t>(lhs * rhs);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::idiv>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		auto r = static_cast<int64_t>(registers[opr.rhs].u64);
		if (unlikely(r == 0))
			throw DivideByZero();
		registers[opr.lvid].u64 = static_cast<uint64_t>(static_cast<int64_t>(registers[opr.lhs].u64) / r);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::eq>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 == registers[opr.rhs].u64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::neq>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 != registers[opr.rhs].u64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::lt>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 < registers[opr.rhs].u64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::lte>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 <= registers[opr.rhs].u64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::ilt>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 < registers[opr.rhs].i64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::ilte>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 <= registers[opr.rhs].i64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::gt>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 > registers[opr.rhs].u64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::gte>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].u64 >= registers[opr.rhs].u64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::igt>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 > registers[opr.rhs].i64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::igte>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].i64 >= registers[opr.rhs].i64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::flt>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 < registers[opr.rhs].f64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::flte>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 <= registers[opr.rhs].f64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fgt>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 > registers[opr.rhs].f64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fgte>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = (registers[opr.lhs].f64 >= registers[opr.rhs].f64) ? 1 : 0;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::opand>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 & registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::opor>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 | registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::opxor>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 ^ registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::opmod>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = registers[opr.lhs].u64 % registers[opr.rhs].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::push>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		parameters[paramCount++].u64 = registers[opr.lvid].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::ret>& opr) {
		printOpcode(opr);
		assert(opr.lvid == 0 or opr.lvid < dbg.registerCount()); // lvid can be null
		returnFromCurrentFunc(registers[opr.lvid].u64);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::store>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid] = registers[opr.source];
	}

	void visit(const ir::isa::Operand<ir::isa::Op::storeText>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		auto cstr = ircode.get().stringrefs[opr.text].c_str();
		registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(cstr);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::storeConstant>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = opr.value.u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::classdefsizeof>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		assert(map.findAtom(opr.type) != nullptr);
		registers[opr.lvid].u64 = map.findAtom(opr.type)->runtimeSizeof();
	}

	void visit(const ir::isa::Operand<ir::isa::Op::call>& opr) {
		validateLvids(opr);
		printOpcode(opr);
		call(opr.lvid, opr.ptr2func, opr.instanceid);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fieldset>& opr) {
		assert(opr.self < dbg.registerCount());
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.self].u64);
		allocator.validate(object, opr.self);
		object[1 + opr.var] = registers[opr.lvid].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::fieldget>& opr) {
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.self].u64);
		allocator.validate(object, opr.self);
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
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		allocator.validate(object, opr.lvid);
		++(object[0]); // +ref
	}

	void visit(const ir::isa::Operand<ir::isa::Op::unref>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		allocator.validate(object, opr.lvid);
		if (0 == --(object[0])) // -unref
			destroy(object, opr.atomid);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::stackalloc>& opr) {
		printOpcode(opr);
		validateLvids(opr);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memalloc>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		size_t size = [&]() -> size_t {
			if (sizeof(size_t) == sizeof(uint64_t))
				return static_cast<size_t>(registers[opr.regsize].u64);
			auto request = registers[opr.regsize].u64;
			if (std::numeric_limits<size_t>::max() <= request)
				throw std::bad_alloc();
			return static_cast<size_t>(request);
		}();
		size += config::extraObjectSize;
		uint64_t* pointer = (uint64_t*) allocator.allocate(size, opr.lvid);
		pointer[0] = 0;
		registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(pointer);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memrealloc>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		size_t oldsize = static_cast<size_t>(registers[opr.oldsize].u64);
		size_t newsize = static_cast<size_t>(registers[opr.newsize].u64);
		oldsize += config::extraObjectSize;
		newsize += config::extraObjectSize;
		if (object)
			allocator.validate(object, opr.lvid);
		auto* newptr = (uint64_t*) allocator.reallocate(object, oldsize, newsize, opr.lvid);
		registers[opr.lvid].u64 = reinterpret_cast<uintptr_t>(newptr);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memfree>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		if (object) {
			allocator.validate(object, opr.lvid);
			size_t size = static_cast<size_t>(registers[opr.regsize].u64);
			size += config::extraObjectSize;
			allocator.deallocate(object, size);
		}
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memcheckhold>& opr) {
		uint64_t* ptr = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t size = registers[opr.size].u64 + config::extraObjectSize;
		allocator.hold(ptr, size, opr.lvid);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memfill>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		uint8_t pattern = static_cast<uint8_t>(registers[opr.pattern].u64);
		memset(object, pattern, size);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memcopy>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t* src = reinterpret_cast<uint64_t*>(registers[opr.srclvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		memcpy(object, src, size);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memmove>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t* src = reinterpret_cast<uint64_t*>(registers[opr.srclvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		memmove(object, src, size);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::memcmp>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		uint64_t* src = reinterpret_cast<uint64_t*>(registers[opr.srclvid].u64);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		int cmp = memcmp(object, src, size);
		registers[opr.regsize].u64 = (cmp == 0) ? 0 : ((cmp < 0) ? 2 : 1);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::cstrlen>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		auto* cstring = reinterpret_cast<const char*>(registers[opr.ptr].u64);
		size_t clen = cstring ? strlen(cstring) : 0u;
		registers[opr.lvid].u64 = clen;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::load_u64>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = *(reinterpret_cast<uint64_t*>(registers[opr.ptrlvid].u64));
	}

	void visit(const ir::isa::Operand<ir::isa::Op::load_u32>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = *(reinterpret_cast<uint32_t*>(registers[opr.ptrlvid].u64));
	}

	void visit(const ir::isa::Operand<ir::isa::Op::load_u8>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		registers[opr.lvid].u64 = *(reinterpret_cast<uint8_t*>(registers[opr.ptrlvid].u64));
	}

	void visit(const ir::isa::Operand<ir::isa::Op::store_u64>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		*(reinterpret_cast<uint64_t*>(registers[opr.ptrlvid].u64)) = registers[opr.lvid].u64;
	}

	void visit(const ir::isa::Operand<ir::isa::Op::store_u32>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		*(reinterpret_cast<uint32_t*>(registers[opr.ptrlvid].u64)) = static_cast<uint32_t>(registers[opr.lvid].u64);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::store_u8>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		*(reinterpret_cast<uint8_t*>(registers[opr.ptrlvid].u64)) = static_cast<uint8_t>(registers[opr.lvid].u64);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::opassert>& opr) {
		printOpcode(opr);
		validateLvids(opr);
		if (unlikely(registers[opr.lvid].u64 == 0))
			throw Assert();
	}

	void visit(const ir::isa::Operand<ir::isa::Op::jzraise>& opr) {
		printOpcode(opr);
		if (not unwindRaisedError)
			gotoLabel(opr.label);
	}

	void visit(const ir::isa::Operand<ir::isa::Op::jmperrhandler>& opr) {
		if (opr.atomid == raisedErrorAtomid or opr.atomid == 0) {
			assert(raisedError != nullptr);
			if (opr.atomid != 0) {
				registers[opr.label + 1].u64 = reinterpret_cast<uint64_t>(raisedError);
			}
			else {
				auto atom = map.findAtom(raisedErrorAtomid);
				Atom* dtor = nullptr;
				atom->findFuncAtom(dtor, "^dispose");
				if (unlikely(dtor == nullptr))
					throw InvalidDtor(*atom);
				destroy(reinterpret_cast<uint64_t*>(raisedError), dtor->atomid);
			}
			raisedError = nullptr;
			unwindRaisedError = false;
			gotoLabel(opr.label);
		}
	}

	NYVM_NOINLINE void visit(const ir::isa::Operand<ir::isa::Op::raise>& opr) {
		raisedError = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
		raisedErrorAtomid = opr.atomid;
		if (opr.label == 0) {
			unwindRaisedError = true;
			returnFromCurrentFunc();
		}
		else {
			unwindRaisedError = false;
			registers[opr.label + 1].u64 = reinterpret_cast<uint64_t>(raisedError);
			gotoLabel(opr.label);
		}
	}

	void visit(const ir::isa::Operand<ir::isa::Op::comment>&) {}
	void visit(const ir::isa::Operand<ir::isa::Op::scope>&) {}
	void visit(const ir::isa::Operand<ir::isa::Op::end>&) {}
	void visit(const ir::isa::Operand<ir::isa::Op::nop>&) {}

	template<ir::isa::Op O> void visit(const ir::isa::Operand<O>& opr) {
		printOpcode(opr);
		(void) opr;
		throw UnexpectedOpcode(ny::ir::isa::opname(O));
	}
};

void Executor::destroy(uint64_t* object, uint32_t dtorid) {
	auto& dtor = *map.findAtom(dtorid);
	auto* classobject = dtor.parent;
	assert(classobject != nullptr);
	uint64_t classsizeof = classobject->runtimeSizeof();
	classsizeof += config::extraObjectSize;
	paramCount = 1;
	parameters[0].u64 = reinterpret_cast<uint64_t>(object); // self
	uint32_t instanceid = 0; // always only one version of the dtor
	call(0, dtor.atomid, instanceid);
	allocator.deallocate(object, static_cast<size_t>(classsizeof));
}

inline uint64_t Executor::entrypoint(uint32_t atomfunc, uint32_t instanceid) {
	constexpr uint32_t retlvid = 1;
	dbg.registerCount(2);
	Register localregisters[2];
	registers = &localregisters[0];
	call(retlvid, atomfunc, instanceid);
	return localregisters[retlvid].u64;
}

void Executor::call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid) {
	assert(retlvid == 0 or retlvid < dbg.registerCount());
	if (printOpcodes) {
		std::cout << "== ny:vm >>  registers before call\n";
		for (uint32_t r = 0; r != dbg.registerCount(); ++r)
			std::cout << "== ny:vm >>     reg %" << r << " = " << registers[r].u64 << "\n";
		std::cout << "== ny:vm >> " << (++dbg.calldepth) << " entering func atom:" << atomfunc;
		std::cout << ", instance: " << instanceid << '\n';
	}
	// save the current stack frame
	auto* storestackptr = registers;
	auto storeircode = std::cref(ircode);
	auto* storecursor = cursor;
	auto storestckfrmsize = dbg.registerCount();
	auto labelid = upperLabelID;
	uint32_t memcheckPreviousAtomid = allocator.tracker.atomid();
	stacktrace.push(atomfunc, instanceid);
	// call
	Register ret = ([&](const ir::Sequence& callee) {
		retval.u64 = 0;
		const uint32_t framesize = callee.at<ir::isa::Op::stacksize>(0).add;
		assert(framesize < 1024 * 1024);
		dbg.registerCount(framesize);
		assert(callee.at<ir::isa::Op::stacksize>(0).opcode == (uint32_t) ir::isa::Op::stacksize);
		registers = stack.push(framesize);
		registers[0].u64 = 0;
		ircode = std::cref(callee);
		upperLabelID = 0;
		for (uint32_t i = 0; i != paramCount; ++i)
			registers[i + 2].u64 = parameters[i].u64; // 2-based
		paramCount = 0;
		callee.each(*this, 1); // offset: 1, avoid blueprint pragma
		stack.pop(framesize);
		return retval;
	})(map.ircode(atomfunc, instanceid));
	// restore the previous stack frame and store the result of the call
	upperLabelID = labelid;
	registers = storestackptr;
	registers[retlvid] = ret;
	ircode = std::cref(storeircode);
	cursor = storecursor;
	dbg.registerCount(storestckfrmsize);
	stacktrace.pop();
	allocator.tracker.atomid(memcheckPreviousAtomid);
	if (printOpcodes) {
		std::cout << "== ny:vm << " << dbg.calldepth << " returned from func call\n";
		--dbg.calldepth;
	}
}

} // namespace

Thread::Thread(Machine& machine)
	: machine(machine) {
	capi.userdata = machine.opts.userdata;
	memcpy(&capi.allocator, &machine.opts.allocator, sizeof(capi.allocator));
	memcpy(&capi.cout, &machine.opts.cout, sizeof(capi.cout));
	memcpy(&capi.cerr, &machine.opts.cout, sizeof(capi.cerr));
	capi.program = ny::Program::pointer(machine.program);
	capi.internal = this;
	capi.io_resolve = io_resolve;
	capi.io_get_cwd = io_get_cwd;
	capi.io_set_cwd = io_set_cwd;
	capi.io_add_mountpoint = io_add_mountpoint;
}

uint64_t Thread::execute(uint32_t atomid, uint32_t instanceid) {
	try {
		auto& map = machine.program.compdb->cdeftable.atoms;
		auto& ircode = map.ircode(atomid, instanceid);
		Executor executor{*this, ircode};
		executor.stacktrace.push(atomid, instanceid);
		executor.entrypoint(atomid, instanceid);
		return 0;
	}
	catch (const InvalidLabel&) {
	}
	catch (const DivideByZero&) {
	}
	catch (const Assert&) {
	}
	catch (const UnexpectedOpcode&) {
	}
	catch (const InvalidDtor&) {
	}
	catch (const ICE&) {
	}
	catch (...) {
	}
	return 120;
}

} // namespace vm
} // namespace ny
