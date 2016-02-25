#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::classdefsizeof>& operands)
	{
		if (canGenerateCode())
		{
			if (not frame->verify(operands.type))
				return frame->invalidate(operands.lvid);

			auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame->atomid, operands.type});
			if (not cdef.isBuiltinOrVoid())
			{
				auto* atom = cdeftable.findClassdefAtom(cdef);
				if (unlikely(nullptr == atom))
					return (void)(ICE() << "invalid atom for sizeof operator");

				out.emitSizeof(operands.lvid, atom->atomid);
			}
			else
			{
				uint64_t size = nany_type_sizeof(cdef.kind);
				out.emitStore_u64(operands.lvid, size);
			}
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
