#include "sequence.h"
#include <cstdlib>
#include "details/ir/isa/printer.inc.hpp"
#include "details/atom/atom.h"

using namespace Yuni;



namespace Nany
{
namespace IR
{


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





} // namespace IR
} // namespace Nany
