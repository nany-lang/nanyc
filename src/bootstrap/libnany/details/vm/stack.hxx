#pragma once
#include "stack.h"
#include <cstring>



namespace Nany
{
namespace VM
{


	inline DataRegister* Stack::push(uint32_t count)
	{
		#if NANY_VM_STACK_TRACES != 0
		++frameCount;
		dump(count);
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
		return registers;
	}


	inline void Stack::pop(uint32_t count)
	{
		#if NANY_VM_STACK_TRACES != 0
		--frameCount;
		#endif

		current->cursor -= count;
		if (YUNI_UNLIKELY((current->remains += count) == current->capacity))
			popChunk();
	}




} // namespace VM
} // namespace Nany
