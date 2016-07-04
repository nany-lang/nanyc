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


	void Sequence::grow(uint32_t instrCount)
	{
		auto newcapa = pCapacity;
		do { newcapa += 1000; } while (newcapa < instrCount);
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


	void Sequence::print(YString& out, const AtomMap* atommap, uint32_t offset) const
	{
		using namespace IR::ISA;
		out.reserve(out.size() + (pSize * 100)); // arbitrary
		Printer<String> printer{out, *this};
		printer.atommap = atommap;
		printer.offset = offset;
		each(printer, offset);
	}




	namespace // anonymous
	{

		struct WalkerIncreaseLVID final
		{
			WalkerIncreaseLVID(IR::Sequence& sequence, uint32_t inc, uint32_t greaterThan)
				: greaterThan(greaterThan)
				, inc(inc)
				, sequence(sequence)
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
						sequence.moveCursorFromBlueprintToEnd(*cursor);
						break;
					default:
						operands.eachLVID(*this);
						break;
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
				// ask to the opcode datatype to update its own lvid
				// (see operator() below)
				operands.eachLVID(*this);
			}

			inline void operator () (uint32_t& lvid) const
			{
				if (lvid > greaterThan)
					lvid += inc;
			}
			inline void operator () (uint32_t& lvid1, uint32_t& lvid2) const
			{
				if (lvid1 > greaterThan)
					lvid1 += inc;
				if (lvid2 > greaterThan)
					lvid2 += inc;
			}
			inline void operator () (uint32_t& lvid1, uint32_t& lvid2, uint32_t& lvid3) const
			{
				if (lvid1 > greaterThan)
					lvid1 += inc;
				if (lvid2 > greaterThan)
					lvid2 += inc;
				if (lvid3 > greaterThan)
					lvid3 += inc;
			}

			uint32_t greaterThan;
			uint32_t inc;
			uint32_t depth = 0;
			Instruction** cursor = nullptr;
			Sequence& sequence;
		};

	} // anonymous namespace

	void Sequence::increaseAllLVID(uint32_t inc, uint32_t greaterThan, uint32_t offset)
	{
		assert(inc > 0 and "this method should not be called if nothing to do");

		WalkerIncreaseLVID walker{*this, inc, greaterThan};
		each(walker, offset);
	}




} // namespace IR
} // namespace Nany
