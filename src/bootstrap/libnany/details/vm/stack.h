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

		/*!
		** \brief Push a new frame and allocates registers
		*/
		DataRegister* push(uint32_t count);

		/*!
		** \brief Remove the last frame
		*/
		void pop(uint32_t count);


	private:
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
