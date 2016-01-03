#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	inline void ProgramBuilder::adjustSettingsNewFuncdefOperator(const AnyString& name)
	{
		assert(name.size() >= 2);
		switch (name[1])
		{
			case 'd':
			{
				if (name == "^default-new")
				{
					generateClassVarsAutoInit = true; // same as '^new'
					break;
				}
				break;
			}

			case 'n':
			{
				if (name == "^new")
				{
					generateClassVarsAutoInit = true; // same as '^default-new'
					break;
				}
				break;
			}

			case 'o':
			{
				if (name == "^obj-dispose")
				{
					generateClassVarsAutoRelease = true;
					break;
				}
				if (name == "^obj-clone")
				{
					generateClassVarsAutoClone = true;
					break;
				}
				break;
			}
		}
	}


	bool ProgramBuilder::pragmaBlueprint(const IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		// reset
		lastPushedNamedParameters.clear();
		lastPushedIndexedParameters.clear();

		generateClassVarsAutoInit = false;
		generateClassVarsAutoRelease = false;
		lastOpcodeStacksizeOffset = (uint32_t) -1;

		assert(layerDepthLimit > 0);
		--layerDepthLimit;

		uint32_t atomid = operands.value.blueprint.atomid;
		AnyString atomname = currentProgram.stringrefs[operands.value.blueprint.name];

		if (not atomStack.empty() and canGenerateCode())
		{
			if ((IR::ISA::Pragma) operands.pragma == IR::ISA::Pragma::blueprintfuncdef)
				out.emitBlueprintFunc(atomname, atomid);
			else
				out.emitBlueprintClass(atomname, atomid);
		}

		auto* atom = cdeftable.atoms().findAtom(atomid);
		if (unlikely(nullptr == atom))
		{
			complainOperand(reinterpret_cast<const IR::Instruction&>(operands), "invalid atom");
			return false;
		}

		atomStack.emplace_back(*atom);
		atomStack.back().blueprintOpcodeOffset = currentProgram.offsetOf(**cursor);

		if ((IR::ISA::Pragma) operands.pragma == IR::ISA::Pragma::blueprintfuncdef)
		{
			if (atomname[0] == '^') // operator !
			{
				// some special actions must be triggered according the operator name
				adjustSettingsNewFuncdefOperator(atomname);
			}
		}

		assert(not atomStack.empty() and "invalid atom stack");
		return true;
	}


	void ProgramBuilder::pragmaBodyStart()
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


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		if (unlikely(not (operands.pragma < static_cast<uint32_t>(IR::ISA::Pragma::max))))
			return (void)(ICE() << "invalid pragma");

		switch ((IR::ISA::Pragma) operands.pragma)
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

			case IR::ISA::Pragma::blueprintparam:
			{
				// -- function parameter
				assert(not atomStack.empty() and "invalid atom stack");
				uint32_t sid  = operands.value.param.name;
				uint32_t lvid = operands.value.param.lvid;
				cdeftable.substitute(lvid).qualifiers.ref = false;
				declareNamedVariable(currentProgram.stringrefs[sid], lvid, false);
				break;
			}

			case IR::ISA::Pragma::blueprintparamself:
			{
				// -- function parameter
				// -- with automatic variable assignment for operator new
				// example: operator new (self varname) {}
				assert(not atomStack.empty() and "invalid atom stack");
				auto& frame = atomStack.back();
				if (unlikely(not frame.atom.isClassMember()))
				{
					error() << "automatic variable assignment is only allowed in class operator 'new'";
					break;
				}

				if (!frame.selfParameters.get())
					frame.selfParameters = std::make_unique<decltype(frame.selfParameters)::element_type>();

				uint32_t sid  = operands.value.param.name;
				uint32_t lvid = operands.value.param.lvid;
				AnyString varname = currentProgram.stringrefs[sid];
				(*frame.selfParameters)[varname].first = lvid;
				break;
			}

			case IR::ISA::Pragma::blueprintfuncdef:
			case IR::ISA::Pragma::blueprintclassdef:
			{
				pragmaBlueprint(operands);
				break;
			}

			case IR::ISA::Pragma::bodystart:
			{
				pragmaBodyStart();
				break;
			}

			case IR::ISA::Pragma::blueprintvar:
			{
				// update the type of the variable member
				assert(not atomStack.empty());
				auto& frame = atomStack.back();
				if (frame.atom.isClass())
				{
					uint32_t sid  = operands.value.vardef.name;
					const AnyString& varname = currentProgram.stringrefs[sid];

					Atom* atom = nullptr;
					if (1 != frame.atom.findVarAtom(atom, varname))
					{
						ICE() << "unknown variable member '" << varname << "'";
						return;
					}
					atom->returnType.clid.reclass(frame.atomid, operands.value.vardef.lvid);
				}
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
					*cursor = &currentProgram.at(startOffset + count - 1 - 1);
				}
				break;
			}

			case IR::ISA::Pragma::visibility:
			{
				assert(not atomStack.empty() and "invalid atom stack");
				break;
			}

			case IR::ISA::Pragma::shortcircuit:
			{
				break;
			}

			case IR::ISA::Pragma::namespacedef:
			case IR::ISA::Pragma::unknown:
			case IR::ISA::Pragma::max:
			default:
			{
				ICE() << "invalid pragma value " << reinterpret_cast<void*>(operands.pragma);
				break;
			}
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
