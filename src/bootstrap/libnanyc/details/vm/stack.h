#pragma once
#include "types.h"
#include "nany/nany.h"
#include "details/context/build.h"

#define NANY_VM_STACK_TRACES 0


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
		Stack(const Stack&) = delete;
		~Stack();

		/*!
		** \brief Push a new frame and allocates registers
		*/
		DataRegister* push(uint32_t count);

		/*!
		** \brief Remove the last frame
		*/
		void pop(uint32_t count);

		//! Operator =
		Stack& operator = (const Stack&) = delete;


	private:
		void pushNewChunk(uint32_t count);
		void popChunk();
		void dump(const AnyString& action, uint32_t count) const;

	private:
		static_assert(sizeof(DataRegister) == sizeof(uint64_t), "invalid register size");

		struct Chunk
		{
			static constexpr uint32_t blockSizeWanted = 8192u;

			//! The maximum number of bytes that would fit in 2 pages block
			static constexpr uint32_t blockmax =
				static_cast<uint32_t>((blockSizeWanted - sizeof(void*) * 4) / sizeof(DataRegister));

			uint32_t remains;
			uint32_t capacity;
			Chunk* previous;
			DataRegister* cursor;

			DataRegister block[blockmax]; // [capacity]
			// warning: the size of this struct might be bigger than expected
			// the real size of 'block' is 'capacity', which can be greater than 'blockmax'
			// to accept legit big stack requests
		};

		Chunk* current = nullptr;
		Chunk* reserve = nullptr;

		#if NANY_VM_STACK_TRACES != 0
		uint32_t frameCount = 0u;
		uint32_t stacksize = 0u;
		#endif

		Build& build;
	};



} // namespace VM
} // namespace Nany

#include "stack.hxx"
