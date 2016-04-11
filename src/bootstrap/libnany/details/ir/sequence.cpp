#include "sequence.h"
#include <cstdlib>
#include "details/ir/isa/printer.inc.hpp"
#include "details/atom/atom.h"

using namespace Yuni;



namespace Nany
{
namespace IR
{

	void Sequence::moveCursorFromBlueprintToEnd(const Instruction*& cursor) const
	{
		assert((*cursor).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::blueprint));
		if ((*cursor).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::blueprint))
		{
			// next opcode, which should be blueprint.size
			(cursor)++;

			// getting the size and moving the cursor
			auto& blueprintsize = (*cursor).to<IR::ISA::Op::pragma>();
			assert(blueprintsize.opcode == (uint32_t) IR::ISA::Op::pragma);
			assert(blueprintsize.value.blueprintsize >= 2);

			cursor += blueprintsize.value.blueprintsize - 2; // -2: blueprint:class+blueprint:size
			assert((*cursor).opcodes[0] == (uint32_t) IR::ISA::Op::end);
		}
	}


	void Sequence::moveCursorFromBlueprintToEnd(Instruction*& cursor) const
	{
		assert((*cursor).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::blueprint));
		if ((*cursor).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::blueprint))
		{
			// next opcode, which should be blueprint.size
			(cursor)++;

			// getting the size and moving the cursor
			auto& blueprintsize = (*cursor).to<IR::ISA::Op::pragma>();
			assert(blueprintsize.opcode == (uint32_t) IR::ISA::Op::pragma);
			assert(blueprintsize.value.blueprintsize >= 2);

			cursor += blueprintsize.value.blueprintsize - 2; // -2: blueprint:class+blueprint:size
			assert((*cursor).opcodes[0] == (uint32_t) IR::ISA::Op::end);
		}
	}


	void Sequence::clear()
	{
		pSize = 0;
		stringrefs.clear();
	}


	void Sequence::grow(uint32_t N)
	{
		auto newcapa = pCapacity;
		do { newcapa += 1000; } while (newcapa < N);
		pCapacity = newcapa;

		pBody = (Instruction*)::std::realloc(pBody, sizeof(Instruction) * newcapa);
		if (unlikely(nullptr == pBody))
			throw std::bad_alloc();
	}


	void Sequence::shrink()
	{
		if (pSize == 0)
		{
			pCapacity = 0;
			std::free(pBody);
			pBody = nullptr;
		}
		else
		{
			pBody = (Instruction*) std::realloc(pBody, sizeof(Instruction) * pSize);
			if (unlikely(nullptr == pBody))
				throw std::bad_alloc();
		}
	}


	void Sequence::print(Clob& out, const AtomMap* atommap) const
	{
		using namespace ISA;
		out.reserve(out.size() + pSize * 20); // arbitrary
		Printer<Clob> printer{out, *this};
		printer.atommap = atommap;
		each(printer);
	}

	void Sequence::print(YString& out, const AtomMap* atommap) const
	{
		using namespace ISA;
		out.reserve(out.size() + pSize * 20); // arbitrary
		Printer<String> printer{out, *this};
		printer.atommap = atommap;
		each(printer);
	}


	namespace // anonymous
	{

		struct WalkerIncreaseLVID final
		{
			WalkerIncreaseLVID(IR::Sequence& sequence)
				: sequence(sequence)
			{}

			void visit(IR::ISA::Operand<IR::ISA::Op::stacksize>& operands)
			{
				operands.add += inc;
			}

			void visit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
			{
				auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
				switch (kind)
				{
					case IR::ISA::Blueprint::funcdef:
					case IR::ISA::Blueprint::classdef:
					{
						sequence.moveCursorFromBlueprintToEnd(*cursor);
						break;
					}
					default:
					{
						operands.eachLVID(*this);
						break;
					}
				}
			}

			void visit(IR::ISA::Operand<IR::ISA::Op::scope>&)
			{
				++depth;
			}

			void visit(IR::ISA::Operand<IR::ISA::Op::end>&)
			{
				if (depth-- == 0)
					sequence.invalidateCursor(*cursor);
			}

			template<IR::ISA::Op O> void visit(IR::ISA::Operand<O>& operands)
			{
				operands.eachLVID(*this);
			}

			void operator () (uint32_t& lvid) const
			{
				if (lvid > greaterThan)
					lvid += inc;
			}
			void operator () (uint32_t& lvid1, uint32_t& lvid2) const
			{
				if (lvid1 > greaterThan)
					lvid1 += inc;
				if (lvid2 > greaterThan)
					lvid2 += inc;
			}
			void operator () (uint32_t& lvid1, uint32_t& lvid2, uint32_t& lvid3) const
			{
				if (lvid1 > greaterThan)
					lvid1 += inc;
				if (lvid2 > greaterThan)
					lvid2 += inc;
				if (lvid3 > greaterThan)
					lvid3 += inc;
			}

			uint32_t inc;
			uint32_t greaterThan;
			uint32_t depth = 0;
			Instruction** cursor = nullptr;
			Sequence& sequence;
		};

	} // anonymous namespace

	void Sequence::increaseAllLVID(uint32_t inc, uint32_t greaterThan, uint32_t offset)
	{
		if (inc > 0)
		{
			WalkerIncreaseLVID walker{*this};
			walker.inc = inc;
			walker.greaterThan = greaterThan;
			each(walker, offset);
		}
	}




} // namespace IR
} // namespace Nany
