#include "context-runner.h"
#include "details/vm/console.h"

using namespace Yuni;


namespace ny {
namespace vm {


ContextRunner::ContextRunner(Context& context, const ir::Sequence& callee)
	: allocator(context.program.cf.allocator)
	, cf(context.program.cf)
	, context(context)
	, map(context.program.map)
	, sequence(std::cref(callee))
	, userDefinedIntrinsics(ny::ref(context.program.build).intrinsics) {
	// dynamic C calls
	dyncall = dcNewCallVM(4096);
	dcMode(dyncall, DC_CALL_C_DEFAULT);
	// prepare the current context for native C calls
	cfvm.allocator = &allocator;
	cfvm.program = context.program.self();
	cfvm.tctx = context.self();
	cfvm.console = &cf.console;
}


ContextRunner::~ContextRunner() {
	if (dyncall)
		dcFree(dyncall);
	memchecker.printLeaksIfAny(context.cf);
}


void ContextRunner::abortMission() {
	memchecker.releaseAll(allocator); // prevent memory leak reports
	stacktrace.dump(context.cf, map);
	std::longjmp(jump_buffer, 666);
}


void ContextRunner::emitBadAlloc() {
	ny::vm::console::badAlloc(context);
	abortMission();
}


void ContextRunner::emitPointerSizeMismatch(void* object, size_t size) {
	ny::vm::console::invalidPointerSize(context, object,
		/*got*/    size,
		/*expect*/ memchecker.fetchObjectSize(reinterpret_cast<const uint64_t*>(object)));
	abortMission();
}


void ContextRunner::emitAssert() {
	ny::vm::console::assertFailed(context);
	abortMission();
}


void ContextRunner::emitUnexpectedOpcode(const AnyString& name) {
	ny::vm::console::unexpectedOpcode(context, name);
	abortMission();
}


void ContextRunner::emitInvalidIntrinsicParamType() {
	ny::vm::console::invalidIntrinsicParameterType(context);
	abortMission();
}


void ContextRunner::emitInvalidReturnType() {
	ny::vm::console::invalidReturnType(context);
	abortMission();
}


void ContextRunner::emitDividedByZero() {
	ny::vm::console::divisionByZero(context);
	abortMission();
}


void ContextRunner::emitUnknownPointer(void* p) {
	ny::vm::console::unknownPointer(context, p, sequence.get().offsetOf(**cursor));
	abortMission();
}


void ContextRunner::emitLabelError(uint32_t label) {
	ny::vm::console::invalidLabel(context, label, upperLabelID,
		sequence.get().offsetOf(**cursor));
	abortMission();
}


void ContextRunner::destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid) {
	auto& dtor = *map.findAtom(dtorid);
	if (false) {
		std::cout << " .. DESTROY " << (void*) object << " aka '"
			<< dtor.caption() << "' at opc+" << sequence.get().offsetOf(**cursor) << '\n';
		stacktrace.dump(context.cf, map);
		std::cout << '\n';
	}
	auto* classobject = dtor.parent;
	assert(classobject != nullptr);
	uint64_t classsizeof = classobject->runtimeSizeof();
	classsizeof += config::extraObjectSize;
	if (instanceid != (uint32_t) - 1) {
		funcparamCount = 1;
		funcparams[0].u64 = reinterpret_cast<uint64_t>(object); // self
		call(0, dtor.atomid, instanceid);
	}
	if (debugmode)
		memset(object, patternFree, classsizeof);
	deallocate(object, static_cast<size_t>(classsizeof));
	memchecker.forget(object);
}


void ContextRunner::visit(const ir::isa::Operand<ir::isa::Op::intrinsic>& opr) {
	vm_PRINT_OPCODE(opr);
	dcReset(dyncall);
	dcArgPointer(dyncall, &cfvm);
	auto& intrinsic = userDefinedIntrinsics[opr.iid];
	for (uint32_t i = 0; i != funcparamCount; ++i) {
		auto r = funcparams[i];
		switch (intrinsic.params[i]) {
			case nyt_u64:
				dcArgLongLong(dyncall, static_cast<DClonglong>(r.u64));
				break;
			case nyt_i64:
				dcArgLongLong(dyncall, static_cast<DClonglong>(r.i64));
				break;
			case nyt_u32:
				dcArgInt(dyncall, static_cast<DCint>(r.u64));
				break;
			case nyt_i32:
				dcArgInt(dyncall, static_cast<DCint>(r.i64));
				break;
			case nyt_ptr:
				dcArgPointer(dyncall, reinterpret_cast<DCpointer>(r.u64));
				break;
			case nyt_u16:
				dcArgShort(dyncall, static_cast<DCshort>(r.u64));
				break;
			case nyt_i16:
				dcArgShort(dyncall, static_cast<DCshort>(r.i64));
				break;
			case nyt_u8:
				dcArgChar(dyncall, static_cast<DCchar>(r.u64));
				break;
			case nyt_i8:
				dcArgChar(dyncall, static_cast<DCchar>(r.i64));
				break;
			case nyt_f32:
				dcArgFloat(dyncall, static_cast<DCfloat>(r.f64));
				break;
			case nyt_f64:
				dcArgDouble(dyncall, static_cast<DCdouble>(r.f64));
				break;
			case nyt_bool:
				dcArgBool(dyncall, static_cast<DCbool>(r.u64));
				break;
			case nyt_void:
			case nyt_any:
				emitInvalidIntrinsicParamType();
		}
	}
	funcparamCount = 0;
	switch (intrinsic.rettype) {
		case nyt_u64:
			registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallLongLong(dyncall, intrinsic.callback));
			break;
		case nyt_i64:
			registers[opr.lvid].i64 = static_cast<int64_t>(dcCallLongLong(dyncall, intrinsic.callback));
			break;
		case nyt_u32:
			registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallInt(dyncall, intrinsic.callback));
			break;
		case nyt_i32:
			registers[opr.lvid].i64 = static_cast<int64_t>(dcCallInt(dyncall, intrinsic.callback));
			break;
		case nyt_ptr:
			registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(dcCallPointer(dyncall, intrinsic.callback));
			break;
		case nyt_u16:
			registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallShort(dyncall, intrinsic.callback));
			break;
		case nyt_i16:
			registers[opr.lvid].i64 = static_cast<int64_t>(dcCallShort(dyncall, intrinsic.callback));
			break;
		case nyt_u8:
			registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallChar(dyncall, intrinsic.callback));
			break;
		case nyt_i8:
			registers[opr.lvid].i64 = static_cast<int64_t>(dcCallChar(dyncall, intrinsic.callback));
			break;
		case nyt_f32:
			registers[opr.lvid].f64 = static_cast<float>(dcCallFloat(dyncall, intrinsic.callback));
			break;
		case nyt_f64:
			registers[opr.lvid].f64 = static_cast<double>(dcCallDouble(dyncall, intrinsic.callback));
			break;
		case nyt_bool:
			registers[opr.lvid].u64 = (dcCallBool(dyncall, intrinsic.callback) ? 1 : 0);
			break;
		case nyt_void:
			dcCallVoid(dyncall, intrinsic.callback);
			break;
		case nyt_any:
			emitInvalidReturnType();
	}
}


void ContextRunner::visit(const ir::isa::Operand<ir::isa::Op::memalloc>& opr) {
	vm_PRINT_OPCODE(opr);
	ASSERT_LVID(opr.lvid);
	ASSERT_LVID(opr.regsize);
	size_t size;
	if (sizeof(size_t) < sizeof(uint64_t)) {
		// size_t is less than a u64 (32bits platform probably)
		// so it is possible to ask more than the system can provide
		auto request = registers[opr.regsize].u64;
		if (std::numeric_limits<size_t>::max() <= request)
			return emitBadAlloc();
		size = static_cast<size_t>(request);
	}
	else {
		// 64bits platform
		size = static_cast<size_t>(registers[opr.regsize].u64);
	}
	size += config::extraObjectSize;
	uint64_t* pointer = allocateraw<uint64_t>(size);
	if (unlikely(!pointer))
		return emitBadAlloc();
	if (debugmode)
		memset(pointer, patternAlloc, size);
	pointer[0] = 0;
	registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(pointer);
	memchecker.hold(pointer, size, opr.lvid);
}


void ContextRunner::visit(const ir::isa::Operand<ir::isa::Op::memrealloc>& opr) {
	vm_PRINT_OPCODE(opr);
	ASSERT_LVID(opr.lvid);
	ASSERT_LVID(opr.oldsize);
	ASSERT_LVID(opr.newsize);
	uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
	size_t oldsize = static_cast<size_t>(registers[opr.oldsize].u64);
	size_t newsize = static_cast<size_t>(registers[opr.newsize].u64);
	oldsize += config::extraObjectSize;
	newsize += config::extraObjectSize;
	if (object) {
		vm_CHECK_POINTER(object, opr);
		if (unlikely(not memchecker.checkObjectSize(object, static_cast<size_t>(oldsize))))
			return emitPointerSizeMismatch(object, oldsize);
		memchecker.forget(object);
	}
	auto* newptr = (uint64_t*) allocator.reallocate(&allocator, object, oldsize, newsize);
	registers[opr.lvid].u64 = reinterpret_cast<uintptr_t>(newptr);
	if (newptr) {
		// the nointer has been successfully reallocated - keeping a reference
		memchecker.hold(newptr, newsize, opr.lvid);
	}
	else {
		// `realloc` may not release the input pointer if the allocation failed
		deallocate(object, oldsize);
		emitBadAlloc();
	}
}


void ContextRunner::visit(const ir::isa::Operand<ir::isa::Op::memfree>& opr) {
	vm_PRINT_OPCODE(opr);
	ASSERT_LVID(opr.lvid);
	ASSERT_LVID(opr.regsize);
	uint64_t* object = reinterpret_cast<uint64_t*>(registers[opr.lvid].u64);
	if (object) {
		vm_CHECK_POINTER(object, opr);
		size_t size = static_cast<size_t>(registers[opr.regsize].u64);
		size += config::extraObjectSize;
		if (unlikely(not memchecker.checkObjectSize(object, static_cast<size_t>(size))))
			return emitPointerSizeMismatch(object, size);
		if (debugmode)
			memset(object, patternFree, size);
		deallocate(object, size);
		memchecker.forget(object);
	}
}


uint64_t ContextRunner::invoke(const ir::Sequence& callee) {
	const uint32_t framesize = callee.at<ir::isa::Op::stacksize>(0).add;
	#ifndef NDEBUG
	assert(framesize < 1024 * 1024);
	registerCount = framesize;
	assert(callee.at<ir::isa::Op::stacksize>(0).opcode == (uint32_t) ir::isa::Op::stacksize);
	#endif
	registers = stack.push(framesize);
	registers[0].u64 = 0;
	sequence = std::cref(callee);
	upperLabelID = 0;
	// retrieve parameters for the func
	for (uint32_t i = 0; i != funcparamCount; ++i)
		registers[i + 2].u64 = funcparams[i].u64; // 2-based
	funcparamCount = 0;

	callee.each(*this, 1); // offset: 1, avoid blueprint pragma
	stack.pop(framesize);
	return retRegister;
}


} // namespace vm
} // namespace ny
