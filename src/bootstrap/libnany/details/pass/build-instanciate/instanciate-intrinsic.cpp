#include "instanciate.h"
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	bool ProgramBuilder::instanciateUserDefinedIntrinsic(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
	{
		bool success = ([&]() -> bool
		{
			AnyString name = currentProgram.stringrefs[operands.intrinsic];
			if (unlikely(name.empty()))
				return (error() << "invalid empty intrinsic name");

			if (unlikely(not lastPushedNamedParameters.empty()))
				return complainIntrinsicWithNamedParameters(name);


			// trying user-defined intrinsic
			auto* intrinsic = intrinsics.find(name);

			// if not found, it can be a compiler intrinsic
			if (intrinsic == nullptr)
			{
				if (instanciateBuiltinIntrinsic(name, operands.lvid, false))
					return true;

				// intrinsic not found, trying discover mode
				if (context.build.on_intrinsic_discover)
				{
					auto retry = context.build.on_intrinsic_discover(&context, name.c_str(), name.size());
					if (retry != nyfalse)
						intrinsic = intrinsics.find(name);
				}
				if (intrinsic == nullptr)
					return complainUnknownIntrinsic(name);
			}


			if (unlikely(not checkForIntrinsicParamCount(name, intrinsic->paramcount)))
				return false;

			uint count = static_cast<uint>(lastPushedIndexedParameters.size());
			auto& frame = atomStack.back();
			bool hasErrors = false;

			std::cout << intrinsic->print() << std::endl;
			// reset the returned type
			cdeftable.substitute(operands.lvid).kind = intrinsic->rettype;

			for (uint i = 0; i != count; ++i)
			{
				auto& element = lastPushedIndexedParameters[i];
				if (not frame.verify(element.lvid)) // silently ignore error
					return false;

				auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, element.lvid});
				if (unlikely(not cdef.isBuiltin()))
				{
					hasErrors = true;
					complainIntrinsicParameter(name, i, cdef, "a builtin type");
					continue;
				}

				if (canGenerateCode())
					out.emitPush(element.lvid);
			}
			if (unlikely(hasErrors))
				return false;

			if (canGenerateCode())
				out.emitIntrinsic(operands.lvid, name);
			return true;
		})();

		if (not success)
			atomStack.back().lvids[operands.lvid].errorReported = true;
		return success;
	}


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
	{
		if (unlikely(not instanciateUserDefinedIntrinsic(operands)))
		{
			atomStack.back().invalidate(operands.lvid);
			success = false;
		}

		// always remove pushed parameters, whatever the result
		lastPushedNamedParameters.clear();
		lastPushedIndexedParameters.clear();
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
