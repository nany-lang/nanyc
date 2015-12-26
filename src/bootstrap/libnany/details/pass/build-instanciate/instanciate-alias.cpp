#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void ProgramBuilder::declareNamedVariable(const AnyString& name, LVID lvid, bool autoreleased)
	{
		auto& frame = atomStack.back();

		auto& lr    = frame.lvids[lvid];
		LVID found  = frame.findLocalVariable(name);

		if (likely(0 == found))
		{
			lr.scope              = frame.scope;
			lr.userDefinedName    = name;
			lr.origin.file.line   = currentLine;
			lr.origin.file.offset = currentOffset;
			lr.origin.file.url    = currentFilename;
			lr.hasBeenUsed        = false;
			lr.scope              = frame.scope;

			CLID clid{frame.atomid, lvid};
			autoreleased   &= frame.verify(lvid); // suppress spurious errors from previous ones
			lr.autorelease  = autoreleased and canBeAcquired(cdeftable.classdef(clid));
		}
		else
		{
			lr.errorReported = true;
			complainRedeclared(name, found);
		}
	}


	// inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::namealias>& operands)
	// see .h




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
