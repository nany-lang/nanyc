#include "stack.h"
#include <cstdlib>
#include <new>

using namespace Yuni;




namespace Nany
{
namespace VM
{

	Stack::Stack(Build& build)
		: build(build)
	{
		current = build.allocate<Chunk>();
		current->remains  = Chunk::max;
		current->next     = nullptr;
		current->previous = nullptr;
		current->cursor   = &(current->block[0]);
		allocated = current;
	}


	Stack::~Stack()
	{
		auto* c = allocated;
		assert(c != nullptr);
		do
		{
			auto* previous = c->previous;
			build.deallocate(c);
			c = previous;
		}
		while (c);
	}


	void Stack::expandChunk()
	{
		auto* chunk     = build.allocate<Chunk>();
		chunk->next     = nullptr;
		chunk->previous = current;
		chunk->remains  = Chunk::max;
		chunk->cursor   = &(chunk->block[0]);
		current->next   = chunk;
		allocated = chunk;
	}





} // namespace VM
} // namespace Nany
