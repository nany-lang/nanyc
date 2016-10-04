#include "instanciate.h"
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	namespace {


	void blueprintFuncOrClassOrType(SequenceBuilder& seq, const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		seq.pushedparams.clear();
		seq.generateClassVarsAutoInit = false;
		seq.generateClassVarsAutoRelease = false;

		bool bug = (seq.layerDepthLimit == 0); // TODO: determine why this value can be zero with this opcode
		if (not bug)
			--seq.layerDepthLimit;

		uint32_t atomid = operands.atomid;
		uint32_t lvid   = operands.lvid;

		// retrieving the atomid - the atomid may be different from the one requested
		// (class with generic types parameters, anonymous classes...)
		assert(seq.mappingBlueprintAtomID[0] != 0 and "mapping atomid not set");
		assert(seq.mappingBlueprintAtomID[1] != 0 and "mapping atomid not set");
		if (atomid == seq.mappingBlueprintAtomID[0])
			atomid = seq.mappingBlueprintAtomID[1];

		auto* atomptr = seq.cdeftable.atoms().findAtom(atomid);
		if (unlikely(nullptr == atomptr))
		{
			seq.complainOperand(IR::Instruction::fromOpcode(operands), "invalid atom");
			return;
		}
		auto& atom = *atomptr;
		assert(atom.isFunction() or atom.isClass() or atom.isTypeAlias());
		auto& currentSequence = seq.currentSequence;

		// 2 cases can be encountered:
		// - a normal class definition: 'operands.lvid' will be equal to 0
		//   since there is no lvid to update (linked to nothing)
		// - an anonymous class (in the middle of a function for example):
		//   'operands.lvid' will be different from 0. However, when the class
		//   will be instanciated, we will be called again, but without any current 'frame'
		if (lvid == 0 or seq.frame == nullptr)
		{
			seq.pushNewFrame(atom);
			seq.frame->offsetOpcodeBlueprint = currentSequence.offsetOf(**seq.cursor);

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
								seq.generateClassVarsAutoInit = true; // same as '^new'
							else if (atomname == "^dispose")
								seq.generateClassVarsAutoRelease = true;
							break;
						case 'n':
							if (atomname == "^new")
								seq.generateClassVarsAutoInit = true; // same as '^default-new'
							break;
						case 'o':
							if (atomname == "^obj-clone")
								seq.generateClassVarsAutoClone = true;
							break;
					}
				}
			}
		}
		else
		{
			// ignoring completely this blueprint
			currentSequence.moveCursorFromBlueprintToEnd(*seq.cursor);
			if (bug)
				*seq.cursor = &currentSequence.at(currentSequence.offsetOf(**seq.cursor) + 1);

			if (lvid != 0 and atom.isClass())
			{
				// anonymous class
				// The flag Atom::Flags::captureVariables should already be set via 'mapping'
				assert(atom.flags(Atom::Flags::captureVariables));

				Atom* resAtom = seq.instanciateAtomClass(atom); // instanciating the class
				if (unlikely(!resAtom))
				{
					if (seq.frame)
						seq.frame->invalidate(lvid);
					return;
				}

				// updating the attached lvid for automatic type declaration
				seq.cdeftable.substitute(lvid).mutateToAtom(resAtom);
			}
		}
	}


	void blueprintUnit(SequenceBuilder& seq, const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		seq.pushedparams.clear();
		seq.generateClassVarsAutoInit = false;
		seq.generateClassVarsAutoRelease = false;

		uint32_t atomid = operands.atomid;
		assert(atomid != seq.mappingBlueprintAtomID[0] and "mapping for an unit ?");
		auto* atom = seq.cdeftable.atoms().findAtom(atomid);
		if (unlikely(nullptr == atom))
		{
			seq.complainOperand(IR::Instruction::fromOpcode(operands), "invalid unit atom");
			return;
		}

		seq.pushNewFrame(*atom);
		seq.frame->offsetOpcodeBlueprint = seq.currentSequence.offsetOf(**seq.cursor);
	}


	void blueprintParameter(SequenceBuilder& seq, const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		assert(seq.frame != nullptr);
		uint32_t lvid = operands.lvid;
		auto& cdef = seq.cdeftable.substitute(lvid);
		cdef.qualifiers.ref = false;
		bool isvar = (kind == IR::ISA::Blueprint::param);
		cdef.instance = isvar;
		seq.frame->lvids[lvid].synthetic = (not isvar);

		// Do not emit warning for 'unused variable' on template parameters
		if (kind == IR::ISA::Blueprint::gentypeparam)
			seq.frame->lvids[lvid].warning.unused = false;

		// param name
		const auto& name = seq.currentSequence.stringrefs[operands.name];
		// declare the new name as locally accessible
		seq.declareNamedVariable(name, lvid, false);
	}


	void blueprintParamSelf(SequenceBuilder& seq, const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		// -- with automatic variable assignment for operator new
		// example: operator new (self varname) {}
		assert(seq.frame != nullptr);
		auto& frame = *seq.frame;

		if (unlikely(not frame.atom.isClassMember()))
		{
			error() << "automatic variable assignment is only allowed in class operator 'new'";
			return;
		}

		if (!frame.selfParameters.get())
			frame.selfParameters = std::make_unique<decltype(frame.selfParameters)::element_type>();

		uint32_t sid  = operands.name;
		uint32_t lvid = operands.lvid;
		AnyString varname = seq.currentSequence.stringrefs[sid];
		(*frame.selfParameters)[varname].first = lvid;
	}


	void blueprintVardef(SequenceBuilder& seq, const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		// update the type of the variable member
		if (seq.frame != nullptr)
		{
			if (seq.frame->atom.isClass())
			{
				uint32_t sid  = operands.name;
				const AnyString& varname = seq.currentSequence.stringrefs[sid];

				Atom* atom = nullptr;
				if (1 != seq.frame->atom.findVarAtom(atom, varname))
				{
					ice() << "unknown variable member '" << varname << "'";
					return;
				}
				atom->returnType.clid.reclass(seq.frame->atomid, operands.lvid);
			}
		}
		seq.pushedparams.clear();
	}


	} // anonymous namespace




	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		switch (kind)
		{
			case IR::ISA::Blueprint::param: // -- function parameter
			case IR::ISA::Blueprint::gentypeparam:
			{
				blueprintParameter(*this, operands);
				break;
			}
			case IR::ISA::Blueprint::paramself: // -- function parameter
			{
				blueprintParamSelf(*this, operands);
				break;
			}
			case IR::ISA::Blueprint::funcdef:
			case IR::ISA::Blueprint::classdef:
			case IR::ISA::Blueprint::typealias:
			{
				blueprintFuncOrClassOrType(*this, operands);
				break;
			}
			case IR::ISA::Blueprint::vardef:
			{
				blueprintVardef(*this, operands);
				break;
			}
			case IR::ISA::Blueprint::namespacedef:
			{
				// see mapping instead
				break;
			}
			case IR::ISA::Blueprint::unit:
			{
				blueprintUnit(*this, operands);
				break;
			}
		}
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
