#pragma once
#include "stack.h"
#include <cstring>



namespace ny
{
namespace vm
{


	inline DataRegister* Stack::push(uint32_t count)
	{
		#if NANY_vm_STACK_TRACES != 0
		++frameCount;
		dump("push +", count);
		#endif

		// not enough space in the current chunk to allocate N registers
		if (YUNI_UNLIKELY(current->remains < count))
			pushNewChunk(count);

		// the current
		DataRegister* registers = current->cursor;
		if (Yuni::debugmode)
			memset(registers, 0xDE, sizeof(DataRegister) * count);

		current->cursor  += count;
		current->remains -= count;
		assert(registers >= current->block);
		assert(registers < current->block + current->capacity);
		return registers;
	}


	inline void Stack::pop(uint32_t count)
	{
		#if NANY_vm_STACK_TRACES != 0
		--frameCount;
		dump("pop -", count);
		#endif

		current->cursor -= count;
		assert(current->cursor >= current->block);
		assert(current->cursor < current->block + current->capacity);

		if (YUNI_UNLIKELY((current->remains += count) == current->capacity))
			popChunk();
	}


} // namespace vm
} // namespace ny
