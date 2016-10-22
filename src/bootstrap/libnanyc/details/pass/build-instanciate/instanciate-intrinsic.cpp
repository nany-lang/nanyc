#include "instanciate.h"
#include "details/intrinsic/intrinsic-table.h"
#include "instanciate-error.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	bool SequenceBuilder::instanciateUserDefinedIntrinsic(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
	{
		bool success = ([&]() -> bool
		{
			AnyString name = currentSequence.stringrefs[operands.intrinsic];
			if (unlikely(name.empty()))
				return (error() << "invalid empty intrinsic name");

			// named parameters are not accepted
			if (unlikely(not pushedparams.func.named.empty()))
				return complainIntrinsicWithNamedParameters(name);

			// generic type parameters are not accepted
			if (unlikely(not pushedparams.gentypes.indexed.empty() or not pushedparams.gentypes.named.empty()))
				return complainIntrinsicWithGenTypeParameters(name);

			// checking if one parameter was already flag as 'error'
			for (uint32_t i = 0u; i != pushedparams.func.indexed.size(); ++i)
			{
				if (unlikely(not frame->verify(pushedparams.func.indexed[i].lvid)))
					return false;
			}

			// trying user-defined intrinsic
			auto intrinsic = intrinsics.find(name);

			// if not found, this could be a compiler intrinsic
			if (intrinsic == nullptr)
			{
				switch (instanciateBuiltinIntrinsic(name, operands.lvid, false))
				{
					case Tribool::Value::indeterminate:
						break; // not found
					case Tribool::Value::yes:
						return true;
					case Tribool::Value::no:
						return false; // an error has occured
				}

				// intrinsic not found, trying discover mode
				if (build.cf.on_binding_discovery)
				{
					auto retry = build.cf.on_binding_discovery(build.self(), name.c_str(), name.size());
					if (retry == nytrue)
						intrinsic = intrinsics.find(name);
				}
				if (unlikely(intrinsic == nullptr))
					return complain::unknownIntrinsic(name);
			}


			if (unlikely(not checkForIntrinsicParamCount(name, intrinsic->paramcount)))
				return false;

			uint32_t count = static_cast<uint32_t>(pushedparams.func.indexed.size());
			frame->lvids[operands.lvid].synthetic = false;
			bool hasErrors = false;

			// reset the returned type
			cdeftable.substitute(operands.lvid).kind = intrinsic->rettype;

			for (uint32_t i = 0; i != count; ++i)
			{
				auto& element = pushedparams.func.indexed[i];
				if (not frame->verify(element.lvid)) // silently ignore error
					return false;

				auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame->atomid, element.lvid});
				if (unlikely(not cdef.isBuiltin()))
				{
					hasErrors = true;
					complainIntrinsicParameter(name, i, cdef, "a builtin type");
					continue;
				}

				if (canGenerateCode())
					out->emitPush(element.lvid);
			}
			if (unlikely(hasErrors))
				return false;

			if (canGenerateCode())
				out->emitIntrinsic(operands.lvid, nullptr, intrinsic->id);
			return true;
		})();

		if (not success)
			frame->invalidate(operands.lvid);
		return success;
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
	{
		assert(frame != nullptr);
		if (unlikely(not instanciateUserDefinedIntrinsic(operands)))
		{
			frame->invalidate(operands.lvid);
			success = false;
		}

		// always remove pushed parameters, whatever the result
		pushedparams.clear();
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
