#include "stack.h"
#include <cstdlib>
#include <new>

using namespace Yuni;




namespace Nany
{
namespace VM
{

	Stack::Stack(nycontext_t& context)
		: context(context)
	{
		current = (Chunk*) context.memory.allocate(&context, sizeof(Chunk));
		if (YUNI_UNLIKELY(!current))
			throw std::bad_alloc();

		current->remains  = Chunk::max;
		current->next     = nullptr;
		current->previous = nullptr;
		current->cursor   = &(current->block[0]);
		allocated = current;
	}


	Stack::~Stack()
	{
		auto* c = allocated;
		do
		{
			auto* previous = c->previous;
			context.memory.release(&context, c, sizeof(Chunk));
			c = previous;
		}
		while (c);
	}


	void Stack::expandChunk()
	{
		Chunk* chunk = (Chunk*) context.memory.allocate(&context, sizeof(Chunk));
		if (!chunk)
			throw std::bad_alloc();

		chunk->next     = nullptr;
		chunk->previous = current;
		chunk->remains  = Chunk::max;
		chunk->cursor   = &(chunk->block[0]);
		current->next   = chunk;
		allocated = chunk;
	}




} // namespace VM
} // namespace Nany
