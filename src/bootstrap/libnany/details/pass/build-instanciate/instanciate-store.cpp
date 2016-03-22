#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>& operands)
	{
		assert(frame != nullptr);
		frame->lvids[operands.lvid].synthetic = false;
		out.emitStore_u64(operands.lvid, operands.value.u64);
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::store>& operands)
	{
		assert(frame != nullptr);
		frame->lvids[operands.lvid].synthetic = false;

		if (not frame->verify(operands.source))
			return frame->invalidate(operands.lvid);

		auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
		if (cdef.isAny())
		{
			// type propagation
			auto& cdefsrc = cdeftable.classdef(CLID{frame->atomid, operands.source});
			auto& spare = cdeftable.substitute(cdef.clid.lvid());
			spare.import(cdefsrc);
		}
		out.emitStore(operands.lvid, operands.source);
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::storeText>& operands)
	{
		uint32_t sid = out.emitStoreText(operands.lvid, currentSequence.stringrefs[operands.text]);
		auto& lvidinfo = frame->lvids[operands.lvid];
		lvidinfo.synthetic = false;
		lvidinfo.text_sid  = sid;
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
