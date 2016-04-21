#pragma once
#include "types.h"
#include "nany/nany.h"
#include "details/context/build.h"



namespace Nany
{
namespace VM
{

	/*!
	** \brief Stack implementation
	*/
	class Stack final
	{
	public:
		explicit Stack(Build&);
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
		Build& build;
	};



} // namespace VM
} // namespace Nany

#include "stack.hxx"
