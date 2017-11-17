#pragma once
#include "types.h"

#define NANY_vm_STACK_TRACES 0

namespace ny::vm {

//! Stack implementation
class Stack final {
public:
	Stack();
	Stack(const Stack&) = delete;
	~Stack();
	//! Push a new frame and allocates registers
	Register* push(uint32_t count);
	//! Remove the last frame
	void pop(uint32_t count);
	//! Operator =
	Stack& operator = (const Stack&) = delete;


private:
	void pushNewChunk(uint32_t count);
	void popChunk();
	void dump(const AnyString& action, uint32_t count) const;

private:
	static_assert(sizeof(Register) == sizeof(uint64_t), "invalid register size");

	struct Chunk final {
		static constexpr uint32_t blockSizeWanted = 8192u;

		//! The maximum number of bytes that would fit in 2 pages block
		static constexpr uint32_t blockmax =
			static_cast<uint32_t>((blockSizeWanted - sizeof(void*) * 4) / sizeof(Register));

		uint32_t remains;
		uint32_t capacity;
		Chunk* previous;
		Register* cursor;

		Register block[blockmax]; // [capacity]
		// warning: the size of this struct might be bigger than expected
		// the real size of 'block' is 'capacity', which can be greater than 'blockmax'
		// to accept legit big stack requests
	};

	Chunk* current = nullptr;
	Chunk* reserve = nullptr;

#if NANY_vm_STACK_TRACES != 0
	uint32_t frameCount = 0u;
	uint32_t stacksize = 0u;
#endif
};

} // namespace ny::vm

#include "stack.hxx"
