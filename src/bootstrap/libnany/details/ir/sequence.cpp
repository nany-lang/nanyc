#include "sequence.h"
#include <cstdlib>
#include "details/ir/isa/printer.inc.hpp"
#include "details/atom/atom.h"

using namespace Yuni;



namespace Nany
{
namespace IR
{


	Sequence::Sequence()
		: pBody(nullptr, std::free)
	{}

	Sequence::~Sequence()
	{}


	void Sequence::clear()
	{
		pSize = 0;
		stringrefs.clear();
	}


	size_t Sequence::sizeInBytes() const
	{
		return pCapacity * sizeof(Instruction) + stringrefs.inspectMemoryUsage();
	}



	void Sequence::grow(uint32_t N)
	{
		auto newcapa = pCapacity;
		do { newcapa += 1000000; } while (newcapa < N);
		pCapacity = newcapa;

		Instruction* instrs = pBody.release();
		instrs = (Instruction*)::std::realloc(instrs, sizeof(Instruction) * newcapa);
		if (nullptr != instrs)
			pBody.reset(instrs);
		else
			throw std::bad_alloc();
	}


	void Sequence::shrink()
	{
		if (pSize == 0)
		{
			pCapacity = 0;
			pBody.reset(nullptr);
		}
		else
		{
			Instruction* instrs = pBody.release();
			instrs = (Instruction*) std::realloc(instrs, sizeof(Instruction) * pSize);
			if (nullptr != instrs)
				pBody.reset(instrs);
			else
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


	void Sequence::emitDebugfile(const AnyString& filename)
	{
		auto& operands    = emit<ISA::Op::debugfile>();
		operands.filename = stringrefs.ref(filename);
	}





} // namespace IR
} // namespace Nany
