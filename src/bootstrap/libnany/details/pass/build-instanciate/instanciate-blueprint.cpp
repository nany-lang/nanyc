#include "instanciate.h"
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	inline void SequenceBuilder::adjustSettingsNewFuncdefOperator(const AnyString& name)
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
				if (name == "^dispose")
				{
					generateClassVarsAutoRelease = true;
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
				if (name == "^obj-clone")
				{
					generateClassVarsAutoClone = true;
					break;
				}
				break;
			}
		}
	}



	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		switch (kind)
		{
			case IR::ISA::Blueprint::funcdef:
			case IR::ISA::Blueprint::classdef:
			{
				// reset
				pushedparams.clear();
				generateClassVarsAutoInit = false;
				generateClassVarsAutoRelease = false;


				bool bug = (layerDepthLimit == 0);
				if (not bug)
					--layerDepthLimit;

				uint32_t atomid = operands.atomid;

				auto* atom = cdeftable.atoms().findAtom(atomid);
				if (unlikely(nullptr == atom))
				{
					complainOperand(IR::Instruction::fromOpcode(operands), "invalid atom");
					break;
				}

				// 2 cases can be encountered:
				// - a normal class definition: 'operands.lvid' will be equal to 0
				//   since there is no lvid to update (linked to nothing)
				// - an anonymous class (in the middle of a function for example):
				//   'operands.lvid' will be different from 0. However, when the class
				//   will be instanciated, we will be called again, but without any current 'frame'
				if (operands.lvid == 0 or !frame)
				{
					// declare a new class

					AnyString atomname = currentSequence.stringrefs[operands.name];
					if (frame != nullptr and canGenerateCode())
					{
						if (kind == IR::ISA::Blueprint::funcdef)
							out.emitBlueprintFunc(atomname, atomid);
						else
							out.emitBlueprintClass(atomname, atomid);
					}

					// create new frame, and populating the associated variable 'frame'
					pushNewFrame(*atom);
					assert(frame);
					frame->offsetOpcodeBlueprint = currentSequence.offsetOf(**cursor);

					if (kind == IR::ISA::Blueprint::funcdef)
					{
						if (atomname[0] == '^') // operator !
						{
							// some special actions must be triggered according the operator name
							adjustSettingsNewFuncdefOperator(atomname);
						}
					}
				}
				else
				{
					// anonymous class
					// The flag Atom::Flags::captureVariables should already be set via 'mapping'
					assert(atom->flags(Atom::Flags::captureVariables));
					// updating the attached lvid for automatic type declaration
					cdeftable.substitute(operands.lvid).mutateToAtom(atom);

					// ignoring completely this blueprint, so the cursor will be
					// moved to its final corresponding opcode 'end'
					currentSequence.moveCursorFromBlueprintToEnd(*cursor);
					if (bug)
						*cursor = &currentSequence.at(currentSequence.offsetOf(**cursor) + 1);

					bool instok = instanciateAtomClass(*atom); // instanciating the class
					if (unlikely(not instok))
						return;
				}
				break;
			}

			case IR::ISA::Blueprint::param: // -- function parameter
			case IR::ISA::Blueprint::gentypeparam:
			{
				assert(frame != nullptr);
				uint32_t lvid = operands.lvid;
				auto& cdef = cdeftable.substitute(lvid);
				cdef.qualifiers.ref = false;
				bool isvar = (kind == IR::ISA::Blueprint::param);
				cdef.instance = isvar;
				frame->lvids[lvid].synthetic = (not isvar);

				// Do not emit warning for 'unused variable' on template parameters
				if (kind == IR::ISA::Blueprint::gentypeparam)
					frame->lvids[lvid].warning.unused = false;

				// param name
				const auto& name = currentSequence.stringrefs[operands.name];
				// declare the new name as locally accessible
				declareNamedVariable(name, lvid, false);
				break;
			}

			case IR::ISA::Blueprint::paramself: // -- function parameter
			{
				// -- with automatic variable assignment for operator new
				// example: operator new (self varname) {}
				assert(frame != nullptr);
				if (unlikely(not frame->atom.isClassMember()))
				{
					error() << "automatic variable assignment is only allowed in class operator 'new'";
					break;
				}

				if (!frame->selfParameters.get())
					frame->selfParameters = std::make_unique<decltype(frame->selfParameters)::element_type>();

				uint32_t sid  = operands.name;
				uint32_t lvid = operands.lvid;
				AnyString varname = currentSequence.stringrefs[sid];
				(*frame->selfParameters)[varname].first = lvid;
				break;
			}

			case IR::ISA::Blueprint::vardef:
			{
				// update the type of the variable member
				if (frame != nullptr)
				{
					if (frame->atom.isClass())
					{
						uint32_t sid  = operands.name;
						const AnyString& varname = currentSequence.stringrefs[sid];

						Atom* atom = nullptr;
						if (1 != frame->atom.findVarAtom(atom, varname))
						{
							ice() << "unknown variable member '" << varname << "'";
							break;
						}
						atom->returnType.clid.reclass(frame->atomid, operands.lvid);
					}
				}
				pushedparams.clear();
				break;
			}

			case IR::ISA::Blueprint::typealias:
			{
				pushedparams.clear();
				break;
			}

			case IR::ISA::Blueprint::unit:
			{
				// reset
				pushedparams.clear();
				generateClassVarsAutoInit = false;
				generateClassVarsAutoRelease = false;

				uint32_t atomid = operands.atomid;
				auto* atom = cdeftable.atoms().findAtom(atomid);
				if (unlikely(nullptr == atom))
				{
					complainOperand(IR::Instruction::fromOpcode(operands), "invalid unit atom");
					break;
				}

				pushNewFrame(*atom);
				frame->offsetOpcodeBlueprint = currentSequence.offsetOf(**cursor);
				break;
			}

			case IR::ISA::Blueprint::namespacedef:
				break;
		}
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
