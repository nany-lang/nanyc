#include "instanciate.h"
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visitBlueprintFuncOrClassOrType(const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		pushedparams.clear();
		generateClassVarsAutoInit = false;
		generateClassVarsAutoRelease = false;

		bool bug = (layerDepthLimit == 0); // TODO: determine why this value can be zero with this opcode
		if (not bug)
			--layerDepthLimit;

		uint32_t atomid = operands.atomid;
		uint32_t lvid   = operands.lvid;

		// retrieving the atomid - the atomid may be different from the one requested
		// (class with generic types parameters, anonymous classes...)
		assert(mappingBlueprintAtomID[0] != 0 and "mapping atomid not set");
		assert(mappingBlueprintAtomID[1] != 0 and "mapping atomid not set");
		if (atomid == mappingBlueprintAtomID[0])
			atomid = mappingBlueprintAtomID[1];

		auto* atomptr = cdeftable.atoms().findAtom(atomid);
		if (unlikely(nullptr == atomptr))
		{
			complainOperand(IR::Instruction::fromOpcode(operands), "invalid atom");
			return;
		}
		auto& atom = *atomptr;
		assert(atom.isFunction() or atom.isClass() or atom.isTypeAlias());

		// 2 cases can be encountered:
		// - a normal class definition: 'operands.lvid' will be equal to 0
		//   since there is no lvid to update (linked to nothing)
		// - an anonymous class (in the middle of a function for example):
		//   'operands.lvid' will be different from 0. However, when the class
		//   will be instanciated, we will be called again, but without any current 'frame'
		if (lvid == 0 or !frame)
		{
			pushNewFrame(atom);
			frame->offsetOpcodeBlueprint = currentSequence.offsetOf(**cursor);

			auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
			if (kind == IR::ISA::Blueprint::funcdef)
			{
				// some special actions must be triggered according the operator name
				AnyString atomname = currentSequence.stringrefs[operands.name];
				if (atomname.first() == '^') // operator spotted
				{
					assert(atomname.size() >= 2);
					switch (atomname[1])
					{
						case 'd':
							if (atomname == "^default-new")
								generateClassVarsAutoInit = true; // same as '^new'
							else if (atomname == "^dispose")
								generateClassVarsAutoRelease = true;
							break;
						case 'n':
							if (atomname == "^new")
								generateClassVarsAutoInit = true; // same as '^default-new'
							break;
						case 'o':
							if (atomname == "^obj-clone")
								generateClassVarsAutoClone = true;
							break;
					}
				}
			}
		}
		else
		{
			// ignoring completely this blueprint
			currentSequence.moveCursorFromBlueprintToEnd(*cursor);
			if (bug)
				*cursor = &currentSequence.at(currentSequence.offsetOf(**cursor) + 1);

			if (lvid != 0 and atom.isClass())
			{
				// anonymous class
				// The flag Atom::Flags::captureVariables should already be set via 'mapping'
				assert(atom.flags(Atom::Flags::captureVariables));

				Atom* resAtom = instanciateAtomClass(atom); // instanciating the class
				if (unlikely(!resAtom))
				{
					if (frame)
						frame->invalidate(lvid);
					return;
				}

				// updating the attached lvid for automatic type declaration
				cdeftable.substitute(lvid).mutateToAtom(resAtom);
			}
		}
	}


	void SequenceBuilder::visitBlueprintUnit(const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		pushedparams.clear();
		generateClassVarsAutoInit = false;
		generateClassVarsAutoRelease = false;

		uint32_t atomid = operands.atomid;
		assert(atomid != mappingBlueprintAtomID[0] and "mapping for an unit ?");
		auto* atom = cdeftable.atoms().findAtom(atomid);
		if (unlikely(nullptr == atom))
		{
			complainOperand(IR::Instruction::fromOpcode(operands), "invalid unit atom");
			return;
		}

		pushNewFrame(*atom);
		frame->offsetOpcodeBlueprint = currentSequence.offsetOf(**cursor);
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		switch (kind)
		{
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
				return;
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
				return;
			}

			case IR::ISA::Blueprint::funcdef:
			case IR::ISA::Blueprint::classdef:
			case IR::ISA::Blueprint::typealias:
			{
				visitBlueprintFuncOrClassOrType(operands);
				return;
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
				return;
			}

			case IR::ISA::Blueprint::unit:
			{
				visitBlueprintUnit(operands);
				return;
			}

			case IR::ISA::Blueprint::namespacedef:
				// see mapping instead
				break;
		}
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
