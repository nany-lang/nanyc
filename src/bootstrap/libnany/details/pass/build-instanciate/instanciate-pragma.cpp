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
		assert(frame != nullptr);
		auto& atom = frame->atom;
		if (atom.isFunction())
		{
			//
			// cloning all function parameters (only non-ref parameters)
			//
			uint32_t count = atom.parameters.size();
			atom.parameters.each([&](uint32_t i, const AnyString&, const Vardef& vardef)
			{
				// lvid for the given parameter
				uint32_t lvid  = i + 1 + 1; // 1 based, 1: return type
				frame->lvids[lvid].synthetic = false;

				if (not cdeftable.classdef(vardef.clid).qualifiers.ref)
				{
					// a register has already been reserved for cloning parameters
					uint32_t clone = 2 + count + i;

					if (debugmode and canGenerateCode())
						out.emitComment(String{} << "\n ----- ---- deep copy parameter " << i);

					// the new value is not synthetic
					frame->lvids[clone].synthetic = false;

					bool r = instanciateAssignment(*frame, clone, lvid, false, false, true);
					if (unlikely(not r))
						frame->invalidate(clone);

					if (canBeAcquired(lvid))
					{
						frame->lvids[lvid].autorelease = true;
						frame->lvids[clone].autorelease = false;
					}

					if (canGenerateCode())
					{
						out.emitStore(lvid, clone); // register swap
						if (debugmode)
							out.emitComment("--\n");
					}
				}
			});

			// generating some code on the fly
			if (atom.isOperator() and canGenerateCode())
			{
				// variables initialization (for a ctor)
				if (generateClassVarsAutoInit)
				{
					generateClassVarsAutoInit = false;
					generateMemberVarDefaultInitialization();
				}

				// variables destruction (for dtor)
				if (generateClassVarsAutoRelease)
				{
					generateClassVarsAutoRelease = false;
					generateMemberVarDefaultDispose();
				}

				// variables cloning (copy a ctor)
				if (generateClassVarsAutoClone)
				{
					generateClassVarsAutoClone = false;
					generateMemberVarDefaultClone();
				}
			}
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
				if (0 == layerDepthLimit)
				{
					// ignore the current blueprint
					//*cursor += operands.value.blueprintsize;
					assert(frame != nullptr);
					auto startOffset = frame->blueprintOpcodeOffset;
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
				assert(frame != nullptr);
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
