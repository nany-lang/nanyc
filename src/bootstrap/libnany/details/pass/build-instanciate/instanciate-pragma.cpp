#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::pragmaBodyStart()
	{
		assert(not atomStack.empty());

		// cloning function parameters
		auto& frame = atomStack.back();
		assert(frame.atom.isFunction());
		if (frame.atom.isFunction())
		{
			uint32_t count = frame.atom.parameters.size();
			for (uint32_t i = 0; i != count; ++i)
			{
				auto& cdef = cdeftable.classdef(frame.atom.parameters.vardef(i).clid);
				if (not cdef.qualifiers.ref)
				{
					// a register has been reserved for cloning parameters
					uint32_t lvid = i + 1 + 1; // 1 based, 1: return type
					uint32_t clone = 2 + count + i;

					out.emitComment(String{} << "\n -----   ---- deep copy param " << i);
					instanciateAssignment(frame, clone, lvid, false, false, true);

					if (canBeAcquired(lvid))
					{
						frame.lvids[lvid].autorelease = true;
						frame.lvids[clone].autorelease = false;
					}

					// register swap
					out.emitStore(lvid, clone);

					out.emitComment("--\n");
				}
			}
		}

		// generating some code on the fly
		if (generateClassVarsAutoInit)
		{
			generateClassVarsAutoInit = false;
			if (canGenerateCode())
				generateMemberVarDefaultInitialization();
		}
		if (generateClassVarsAutoRelease)
		{
			generateClassVarsAutoRelease = false;
			if (canGenerateCode())
				generateMemberVarDefaultDispose();
		}
		if (generateClassVarsAutoClone)
		{
			generateClassVarsAutoClone = false;
			if (canGenerateCode())
				generateMemberVarDefaultClone();
		}
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		if (unlikely(not (operands.pragma < static_cast<uint32_t>(IR::ISA::Pragma::max))))
			return (void)(ICE() << "invalid pragma");

		auto pragma = static_cast<IR::ISA::Pragma>(operands.pragma);
		switch (pragma)
		{
			case IR::ISA::Pragma::codegen:
			{
				if (operands.value.codegen != 0)
				{
					if (codeGenerationLock > 0) // re-enable code generation
						--codeGenerationLock;
				}
				else
					++codeGenerationLock;
				break;
			}

			case IR::ISA::Pragma::bodystart:
			{
				pragmaBodyStart();
				break;
			}

			case IR::ISA::Pragma::blueprintsize:
			{
				assert(not atomStack.empty() and "invalid atom stack");
				if (0 == layerDepthLimit)
				{
					// ignore the current blueprint
					//*cursor += operands.value.blueprintsize;
					auto startOffset = atomStack.back().blueprintOpcodeOffset;
					auto count = operands.value.blueprintsize;

					if (unlikely(count < 3))
					{
						ICE() << "invalid blueprint size when instanciating atom";
						break;
					}

					// goto the end of the blueprint
					// -2: the final 'end' opcode must be interpreted
					*cursor = &currentSequence.at(startOffset + count - 1 - 1);
				}
				break;
			}

			case IR::ISA::Pragma::visibility:
			{
				assert(not atomStack.empty() and "invalid atom stack");
				break;
			}

			case IR::ISA::Pragma::shortcircuitOpNopOffset:
			{
				shortcircuit.label = operands.value.shortcircuitMetadata.label;
				break;
			}

			case IR::ISA::Pragma::suggest:
			case IR::ISA::Pragma::builtinalias:
			case IR::ISA::Pragma::shortcircuit:
			case IR::ISA::Pragma::unknown:
			case IR::ISA::Pragma::max:
				break;
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
