#include "instanciate.h"
#include "instanciate-error.h"

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

		if (frame->verify(operands.lvid))
		{
			auto& cdef = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
			do
			{
				if (likely(not cdef.isBuiltinOrVoid()))
				{
					auto* atom = cdeftable.findClassdefAtom(cdef);
					if (likely(nullptr != atom))
					{
						if (unlikely(atom->isClass() or atom->isFunction()))
						{
							if (unlikely(atom->isClass() and not atom->classinfo.isInstanciated))
							{
								complain::classNotInstanciated(*atom);
								break;
							}
							// ok the type is an object
							return;
						}
					}
				}

				auto e = (error() << "class or function expected, got '");
				cdef.print(e.data().message, cdeftable, false);
				e << "' instead";

				if (debugmode)
					e << CLID{frame->atomid, operands.lvid};
			}
			while (false);
		}

		frame->invalidate(operands.lvid);
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
