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


	//! Allocate a new variable on the stack and assign a value to it and get the register
	inline uint32_t allocu64(SequenceRef ref, uint32_t lvid, nytype_t type, uint64_t value) {
		ref.sequence.emitStackalloc(lvid, type);
		ir::emit::constantu64(ref, lvid, value);
		return lvid;
	}
	

	//! Allocate a new variable on the stack and assign a value to it and get the register
	inline uint32_t allocf64(SequenceRef ref, uint32_t lvid, nytype_t type, double value) {
		ref.sequence.emitStackalloc(lvid, type);
		ir::emit::constantf64(ref, lvid, value);
		return lvid;
	}


	//! Allocate a new variable on the stack and assign a text to it and get the register
	inline uint32_t alloctext(SequenceRef ref, uint32_t lvid, const AnyString& text) {
		ref.sequence.emitStackalloc(lvid, nyt_ptr);
		ir::emit::constantText(ref, lvid, text);
		return lvid;
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
namespace dbginfo
{
namespace
{


	//! Emit a debug filename opcode
	inline void filename(SequenceRef ref, const AnyString& path) {
		auto& sequence = ref.sequence;
		sequence.emit<ISA::Op::debugfile>().filename = sequence.stringrefs.ref(path);
	}


} // namespace
} // namespace dbginfo
} // namespace emit
} // namespace ir
} // namespace ny
