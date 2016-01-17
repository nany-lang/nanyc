#pragma once
#include "types.h"



namespace Nany
{
namespace VM
{

	/*!
	** \brief Stack implementation
	*/
	class Stack
	{
	public:
		Stack();
		~Stack();

		DataRegister* push(uint32_t count);

		void pop(uint32_t count);

		void expandChunk();


	private:
		struct Chunk
		{
			enum { max = (8192 - sizeof(void*) * 3) / sizeof(DataRegister) };
			DataRegister block[max];
			uint32_t remains;
			DataRegister* cursor;
			Chunk* next;
			Chunk* previous;
		};

		Chunk* current = nullptr;
		Chunk* allocated = nullptr;
	};



} // namespace VM
} // namespace Nany

#include "stack.hxx"
