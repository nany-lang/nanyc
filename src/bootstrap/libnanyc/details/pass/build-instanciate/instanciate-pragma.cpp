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
		assert(not signatureOnly);
		assert(codeGenerationLock == 0 and "any good reason to not generate code ?");

		auto& atom = frame->atom;
		if (not atom.isFunction())
			return;

		bool generateCode = canGenerateCode();
		uint32_t count = atom.parameters.size();
		atom.parameters.each([&](uint32_t i, const AnyString& name, const Vardef& vardef)
		{
			// lvid for the given parameter
			uint32_t lvid  = i + 1 + 1; // 1: return type, 2: first parameter
			assert(lvid < frame->lvids.size());
			// Obviously, the parameters are not synthetic objects
			// but real variables
			frame->lvids[lvid].synthetic = false;

			//
			// Parameters Deep copy (if required)
			//
			if (name[0] != '^')
			{
				// normal input parameter (not captured - does not start with '^')
				// clone it if necessary (only non-ref parameters)

				if (not cdeftable.classdef(vardef.clid).qualifiers.ref)
				{
					if (debugmode and generateCode)
						out->emitComment(String{"----- deep copy parameter "} << i << " aka " << name);

					// a register has already been reserved for cloning parameters
					uint32_t clone = 2 + count + i; // 1: return type, 2: first parameter
					assert(clone < frame->lvids.size());
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

					if (generateCode)
					{
						out->emitStore(lvid, clone); // register swap
						if (debugmode)
							out->emitComment("--\n");
					}
				}
			}
		});

		// generating some code on the fly
		if (atom.isSpecial() /*ctor, operators...*/ and generateCode)
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


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		assert(static_cast<uint32_t>(operands.pragma) < IR::ISA::PragmaCount);

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
				// In 'signature only' mode, we only care about the
				// parameter user types. Everything from this point in unrelevant
				if (signatureOnly)
					return currentSequence.invalidateCursor(*cursor);

				// parameters Deep copy, implicit var auto-init...
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
					assert(frame->offsetOpcodeBlueprint != (uint32_t) -1);
					auto startOffset = frame->offsetOpcodeBlueprint;
					uint32_t count = operands.value.blueprintsize;

					if (unlikely(count < 3))
					{
						ice() << "invalid blueprint size when instanciating atom";
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

			case IR::ISA::Pragma::shortcircuitMutateToBool:
			{
				uint32_t lvid = operands.value.shortcircuitMutate.lvid;
				uint32_t source = operands.value.shortcircuitMutate.source;

				frame->lvids[lvid].synthetic = false;

				if (true)
				{
					auto& instr = *(*cursor - 1);
					assert(instr.opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::stackalloc));
					uint32_t sizeoflvid = instr.to<IR::ISA::Op::stackalloc>().lvid;

					// sizeof
					auto& atombool = *(cdeftable.atoms().core.object[nyt_bool]);
					out->emitSizeof(sizeoflvid, atombool.atomid);

					auto& opc = cdeftable.substitute(lvid);
					opc.mutateToAtom(&atombool);
					opc.qualifiers.ref = true;

					// ALLOC: memory allocation of the new temporary object
					out->emitMemalloc(lvid, sizeoflvid);
					out->emitRef(lvid);
					frame->lvids[lvid].autorelease = true;
					// reset the internal value of the object
					out->emitFieldset(source, /*self*/lvid, 0); // builtin
				}
				else
					out->emitStore(lvid, source);
				break;
			}

			case IR::ISA::Pragma::synthetic:
			{
				uint32_t lvid = operands.value.synthetic.lvid;
				bool onoff = (operands.value.synthetic.onoff != 0);
				frame->lvids[lvid].synthetic = onoff;
				break;
			}

			case IR::ISA::Pragma::suggest:
			case IR::ISA::Pragma::builtinalias:
			case IR::ISA::Pragma::shortcircuit:
			case IR::ISA::Pragma::unknown:
				break;
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
