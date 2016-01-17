#pragma once
#include "stack.h"
#include <cstring>



namespace Nany
{
namespace VM
{


	inline DataRegister* Stack::push(uint32_t count)
	{
		if (count > current->remains)
		{
			if (YUNI_UNLIKELY(current->next == nullptr))
				expandChunk();
			current = current->next;
		}

		DataRegister* registers = current->cursor;
		current->cursor += count;
		current->remains -= count;
		if (Yuni::debugmode)
			memset(registers, 0xDE, sizeof(DataRegister) * count);

		return registers;
	}


	inline void Stack::pop(uint32_t count)
	{
		current->cursor  -= count;
		if ((current->remains += count) == Chunk::max)
			current = current->previous;
	}




} // namespace VM
} // namespace Nany
