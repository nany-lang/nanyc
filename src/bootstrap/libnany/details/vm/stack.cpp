#include "stack.h"
#include <cstdlib>
#include <new>
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace VM
{

	namespace // anonymous
	{

		template<class T>
		static inline uint32_t sizeFromCapacity(uint32_t capacity)
		{
			uint32_t bytes = static_cast<uint32_t>(sizeof(T)); // the Chunk itself
			bytes -= static_cast<uint32_t>(sizeof(T::block));  // minus the pseudo field 'block'
			bytes += static_cast<uint32_t>(capacity * sizeof(DataRegister));
			return bytes;
		}


	} // anonymous namespace


	Stack::Stack(Build& build)
		: build(build)
	{
		pushNewChunk(1); // not null for boundaries checking
		current->remains -= 1u;
	}


	Stack::~Stack()
	{
		free(reserve);

		auto* c = current;
		assert(c != nullptr);
		do
		{
			auto* previous = c->previous;
			free(c);
			c = previous;
		}
		while (c);
	}


	void Stack::dump(const AnyString& action, uint32_t count) const
	{
		if (current)
		{
			std::cout << "== stack == " << action << count << ", current: " << (void*) current
				<< ", remains: " << current->remains << '/' << current->capacity
				#if NANY_VM_STACK_TRACES != 0
				<< ", " << frameCount << " frames"
				<< ", bytes: " << stacksize
				#endif
				<< '\n';
		}
		else
			std::cout << "== stack == <null>\n";
	}


	void Stack::pushNewChunk(uint32_t count)
	{
		#if NANY_VM_STACK_TRACES != 0
		std::cout << "== stack == requires new chunk to increase stack of "
			<< (sizeof(DataRegister) * count) << " bytes\n";
		#endif

		Chunk* chunk;

		if (reserve and count <= reserve->capacity)
		{
			chunk = reserve;
			reserve = nullptr;
			assert(chunk->remains == chunk->capacity);
		}
		else
		{
			uint32_t capacity = Chunk::blockmax;
			while (capacity < count)
				capacity += Chunk::blockSizeWanted;

			uint32_t bytes = sizeFromCapacity<Chunk>(capacity);

			#if NANY_VM_STACK_TRACES != 0
			std::cout << "== stack == allocate new chunk of " << bytes << " bytes\n";
			stacksize += bytes;
			#endif

			chunk = (Chunk*) malloc(bytes);
			chunk->capacity = capacity;
			chunk->remains  = capacity;
			assert(chunk != nullptr);
		}

		chunk->cursor   = chunk->block;
		chunk->previous = current;
		current = chunk;
		assert(count <= chunk->remains);
	}


	void Stack::popChunk()
	{
		auto* previous = current->previous;
		if (!reserve)
		{
			// keep at least one chunk in reserve for next time
			reserve = current;
		}
		else
		{
			#if NANY_VM_STACK_TRACES != 0
			std::cout << "== stack == release chunk\n";
			stacksize -= sizeFromCapacity<Chunk>(current->capacity);
			#endif

			free(current);
		}

		current = previous;
		// 'current' may be null here at the very last scope, when the program stops
	}




} // namespace VM
} // namespace Nany
