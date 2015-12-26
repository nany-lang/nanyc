#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>& operands)
	{
		assert(not atomStack.empty());
		out.emitStoreConstant(operands.lvid, operands.value.u64);
	}


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::store>& operands)
	{
		assert(not atomStack.empty());
		auto& frame = atomStack.back();

		if (not frame.verify(operands.source))
			return frame.invalidate(operands.lvid);

		auto& cdef  = cdeftable.classdef(CLID{frame.atomid, operands.lvid});
		if (cdef.isAny())
		{
			// type propagation
			auto& cdefsrc = cdeftable.classdef(CLID{frame.atomid, operands.source});
			auto& spare = cdeftable.substitute(cdef.clid.lvid());
			spare.import(cdefsrc);
		}
		out.emitStore(operands.lvid, operands.source);
	}


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::storeText>& operands)
	{
		uint32_t sid = out.emitStoreText(operands.lvid, currentProgram.stringrefs[operands.text]);
		auto& frame = atomStack.back();
		frame.lvids[operands.lvid].text_sid = sid;
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
