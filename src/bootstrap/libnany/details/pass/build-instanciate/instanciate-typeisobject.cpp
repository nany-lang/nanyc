#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::typeisobject>& operands)
	{
		assert(frame != nullptr);
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
		if (likely(not cdef.isBuiltinOrVoid()))
		{
			auto* atom = cdeftable.findClassdefAtom(cdef);
			if (likely(nullptr != atom))
				return;
		}

		auto e = (error() << "invalid type: got '");
		cdef.print(e.data().message, cdeftable, false);
		e << "', expected class";
		frame->invalidate(operands.lvid);
	}







} // namespace Instanciate
} // namespace Pass
} // namespace Nany
