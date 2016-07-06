#include "instanciate.h"
#include "libnany-traces.h"
#include <string.h>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::captureVariables(Atom& atom)
	{
		// Try to capture variables from a list of potentiel candidates created by the mapping

		// get the ownership of the container to release it
		assert(atom.candidatesForCapture != nullptr);
		auto candidatesPtr = std::move(atom.candidatesForCapture);

		auto& candidates = *candidatesPtr;
		if (unlikely(candidates.empty()))
			return;

		// The list of variables that can be really captured
		struct CapturedVar final {
			AnyString name;
			CLID clid;
		};
		auto capturedVars = std::make_unique<CapturedVar[]>(candidates.size());

		// keeping only local variables from the proposed list
		uint32_t count = 0;
		{
			if (Config::Traces::capturedVariables)
				trace() << "capturing variables in '" << frame->atom.caption() << '\'';

			ShortString128 newVarname;
			for (auto& varname: candidates)
			{
				uint32_t lvid = frame->findLocalVariable(varname);
				if (lvid != 0)
				{
					auto& element = capturedVars[count++];
					newVarname.clear() << "^trap^" << varname;
					// the new name must be stored somewhere
					element.name = currentSequence.stringrefs.refstr(newVarname);
					element.clid.reclass(frame->atomid, lvid);

					if (Config::Traces::capturedVariables)
						trace() << ".. capturing var: '" << varname << '\'';
				}
				else
				{
					if (Config::Traces::capturedVariables)
						trace() << ".. capturing var: rejected non local identifier '" << varname << '\'';
				}
			}

			if (Config::Traces::capturedVariables)
				trace() << ".. " << count << " captured variables";
		}
		if (count == 0) // nothing to capture
			return;


		assert(atom.opcodes.sequence != nullptr and "invalid empty IR sequence");
		if (unlikely(atom.opcodes.sequence == nullptr))
			return;
		auto& sequence = *atom.opcodes.sequence;

		auto& table = cdeftable.originalTable();

		// Base offset for the new lvid (new captured variables in the class)
		uint32_t startLvid = atom.localVariablesCount + 1; // 1-based
		auto offset = atom.opcodes.offset;

		#ifndef NDEBUG
		auto& blueprint = sequence.at<IR::ISA::Op::blueprint>(offset);
		assert(blueprint.opcode == (uint32_t) IR::ISA::Op::blueprint);
		#endif
		// blueprint size
		++offset;
		#ifndef NDEBUG
		auto& blueprintsize = sequence.at<IR::ISA::Op::pragma>(offset);
		assert(blueprintsize.opcode == (uint32_t) IR::ISA::Op::pragma);
		#endif

		// Updating the opcode stacksize
		++offset;
		auto& stacksize = sequence.at<IR::ISA::Op::stacksize>(offset);
		assert(stacksize.opcode == static_cast<uint32_t>(IR::ISA::Op::stacksize));
		assert(stacksize.add + 1 == startLvid);
		if (unlikely(stacksize.opcode != static_cast<uint32_t>(IR::ISA::Op::stacksize)))
			return (void)(ice() << "capturing variable: stacksize opcode expected");

		// new stack size
		stacksize.add += count;
		atom.localVariablesCount += count;

		table.bulkAppend(atom.atomid, startLvid, count);


		for (uint32_t i = 0; i != count; ++i)
		{
			auto& var = capturedVars[i];

			// new atom for the new variable member
			auto* newVarAtom = table.atoms.createVardef(atom, var.name);
			table.registerAtom(newVarAtom);

			auto& cdef = table.rawclassdef(CLID{atom.atomid, startLvid + i});
			auto& src  = table.classdef(var.clid);
			// the new captured variable is obviously a 'ref' and shares
			// the same type than the original one
			auto* varSrcAtom = table.findClassdefAtom(src);
			if (unlikely(varSrcAtom == nullptr))
				ice() << "invalid atom for captured variable '" << var.name << "'";
			cdef.mutateToAtom(varSrcAtom);
			cdef.qualifiers.ref = true;

			newVarAtom->returnType.clid = cdef.clid;

			// mark the captured variables as used
			assert(var.clid.atomid() == frame->atomid);
			if (var.clid.atomid() == frame->atomid)
				frame->lvids[var.clid.lvid()].warning.unused = false;
		}

		// When instanciating this method, automatically push captured variables
		atom.flags += Atom::Flags::pushCapturedVariables;

		// adding parameters to all constructors
		atom.eachChild([&](Atom& child) -> bool
		{
			if (child.isCtor())
			{
				for (uint32_t i = 0; i != count; ++i)
				{
					auto& var = capturedVars[i];
					CLID clid{atom.atomid, startLvid + i};

					bool success = child.parameters.append(clid, var.name);
					if (unlikely(not success))
					{
						error() << "too many parameters for appending captured variable in "
							<< frame->atom.caption();
					}
				}

				// need extra size for the next instanciation
				child.opcodes.stackSizeExtra = count;
			}
			return true;
		});
	}




	bool SequenceBuilder::pushCapturedVarsAsParameters(const Atom& atomclass)
	{
		atomclass.eachChild([&](Atom& child) -> bool
		{
			if (not child.isMemberVariable())
				return true;
			if (not child.isCapturedVariable())
				return true;

			AnyString varname{child.name(), /*offset*/ (uint32_t)::strlen("^trap^")};
			if (unlikely(varname.empty()))
			{
				ice() << "invalid empty captured variable name";
				return true;
			}

			uint32_t varlvid = frame->findLocalVariable(varname);
			if (unlikely(0 == varlvid))
			{
				error() << "failed to find captured variable '" << varname << "'";
				return true;
			}

			if (not frame->verify(varlvid))
				return true;

			CLID clid{frame->atomid, varlvid};
			overloadMatch.input.params.named.emplace_back(std::make_pair(child.name(), clid));
			return true;
		});
		return true;
	}




	bool SequenceBuilder::identifyCapturedVar(const IR::ISA::Operand<IR::ISA::Op::identify>& operands, const AnyString& name)
	{
		AnyString captureName;
		{
			ShortString128 newname;
			newname << "^trap^" << name;
			captureName = currentSequence.stringrefs.refstr(newname);
		}
		return identify(operands, captureName, false);
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
