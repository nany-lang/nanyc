#include "instanciate.h"
#include "details/context/isolate.h"
#include <memory>
#include "details/reporting/report.h"
#include "details/atom/func-overload-match.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "details/context/context.h"
#include "libnany-traces.h"
#include "instanciate-atom.h"

using namespace Yuni;






namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	SequenceBuilder::SequenceBuilder(Logs::Report report, ClassdefTableView& cdeftable, nycontext_t& context,
		IR::Sequence& out, IR::Sequence& sequence)
		: cdeftable(cdeftable)
		, context(context)
		, intrinsics(((Context*) context.internal)->intrinsics)
		, out(out)
		, currentSequence(sequence)
		, overloadMatch(report, cdeftable)
		, report(report)
	{
		// reduce memory (re)allocations
		multipleResults.reserve(8); // arbitrary value

		pushedparams.func.indexed.reserve(16);
		pushedparams.func.named.reserve(16);
		pushedparams.gentypes.indexed.reserve(8);
		pushedparams.gentypes.named.reserve(8);
	}


	SequenceBuilder::~SequenceBuilder()
	{
		if (Config::Traces::printClassdefTable)
			printClassdefTable();

		auto* frm = frame;
		while (frm)
		{
			auto* previous = frm->previous;
			frm->~AtomStackFrame();
			context.memory.release(&context, frm, sizeof(AtomStackFrame));
			frm = previous;
		}
	}


	void SequenceBuilder::printClassdefTable()
	{
		for (auto* f = frame; f != nullptr; f = f->previous)
			printClassdefTable(report.subgroup(), *f);
	}


	void SequenceBuilder::releaseScopedVariables(int scope, bool forget)
	{
		if (unlikely(!frame))
			return;
		if (not forget and (not canGenerateCode()))
			return;

		// unref in the reverse order
		auto i = static_cast<uint32_t>(frame->lvids.size());

		if (canGenerateCode())
		{
			while (i-- != 0)
			{
				auto& clcvr = frame->lvids[i];
				if (not (clcvr.scope >= scope))
					continue;

				if (clcvr.autorelease)
				{
					//if (not clcvr.userDefinedName.empty())
					//	out.emitComment(String{"unref var "} << clcvr.userDefinedName << " -> %" << i);
					tryUnrefObject(i);
				}

				// forget this variable!
				if (forget)
				{
					if (not clcvr.userDefinedName.empty() and clcvr.warning.unused)
					{
						if (unlikely(not clcvr.hasBeenUsed) and (clcvr.userDefinedName != "self"))
							complainUnusedVariable(*frame, i);
					}

					clcvr.userDefinedName.clear();
					clcvr.scope = -1;
				}
			}
		}
		else
		{
			assert(forget == true);
			while (i-- != 0) // just invalidate everything
			{
				auto& clcvr = frame->lvids[i];
				if (clcvr.scope >= scope)
				{
					clcvr.userDefinedName.clear();
					clcvr.scope = -1;
				}
			}
		}
	}


	uint32_t SequenceBuilder::createLocalVariables(uint32_t count)
	{
		assert(lastOpcodeStacksizeOffset != (uint32_t) -1);
		assert(frame != nullptr);
		assert(count > 0);

		auto& operands = out.at<IR::ISA::Op::stacksize>(lastOpcodeStacksizeOffset);
		uint32_t startOffset = operands.add;
		int scope = frame->scope;

		if (count == 1)
		{
			frame->resizeRegisterCount((++operands.add), cdeftable);
			auto& lvidinfo = frame->lvids[startOffset];
			lvidinfo.scope = scope;
			lvidinfo.synthetic = false;

			if (canGenerateCode())
			{
				lvidinfo.offsetDeclOut = out.opcodeCount();
				out.emitStackalloc(startOffset, nyt_any);
			}
		}
		else
		{
			operands.add += count;
			frame->resizeRegisterCount(operands.add, cdeftable);

			if (canGenerateCode())
			{
				for (uint32_t i = 0; i != count; ++i)
				{
					auto& lvidinfo = frame->lvids[startOffset + i];
					lvidinfo.scope = scope;
					lvidinfo.synthetic = false;
					lvidinfo.offsetDeclOut = out.opcodeCount();
					out.emitStackalloc(startOffset + i, nyt_any);
				}
			}
			else
			{
				for (uint32_t i = 0; i != count; ++i)
				{
					auto& lvidinfo = frame->lvids[startOffset + i];
					lvidinfo.scope = scope;
					lvidinfo.synthetic = false;
				}
			}
		}
		return startOffset;
	}


	void SequenceBuilder::printClassdefTable(Logs::Report trace, const AtomStackFrame& frame) const
	{
		auto entry = trace.trace();

		entry.message.prefix << cdeftable.keyword(frame.atom) << ' ';
		frame.atom.retrieveCaption(entry.message.prefix, cdeftable);
		entry << " - type matrix, after instanciation - atom " << frame.atom.atomid;

		for (uint i = 0; i != frame.localVariablesCount(); ++i)
		{
			auto clid = CLID{frame.atomid, i};
			auto entry = trace.trace();

			if (cdeftable.hasClassdef(clid) or cdeftable.hasSubstitute(clid))
			{
				cdeftable.printClassdef(entry.message.message, clid, cdeftable.classdef(clid));
				entry.message.message.trimRight();

				if (cdeftable.hasSubstitute(clid))
					entry << " (local replacement)";

				if (frame.lvids[i].isConstexpr)
					entry << " (constexpr)";

				if (frame.lvids[i].errorReported)
					entry << " [ERROR]";
			}
			else
			{
				entry << "    " << clid << ": !!INVALID CLID";
			}
		}
	}



	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::inherit>& operands)
	{
		assert(frame != nullptr);
		if (not frame->verify(operands.lhs))
			return;

		switch (operands.inherit)
		{
			case 2: // qualifiers
			{
				auto& spare = cdeftable.substitute(operands.lhs);
				spare.qualifiers = cdeftable.classdef(CLID{frame->atomid, operands.rhs}).qualifiers;
				break;
			}
			default:
			{
				ICE() << "invalid inherit value " << operands.inherit;
			}
		}
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::namealias>& operands)
	{
		const auto& name = currentSequence.stringrefs[operands.name];
		declareNamedVariable(name, operands.lvid);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::debugfile>& operands)
	{
		currentFilename = currentSequence.stringrefs[operands.filename].c_str();
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
	{
		currentLine   = operands.line;
		currentOffset = operands.offset;
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::follow>& operands)
	{
		if (not operands.symlink)
		{
			auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
			auto& spare = cdeftable.substitute(operands.follower);
			spare.import(cdef);
			spare.instance = true;
		}
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::unref>& operands)
	{
		tryUnrefObject(operands.lvid);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::nop>&)
	{
		// duplicate nop as well since they can be used to insert code
		// (for shortcircuit for example)
		out.emitNop();
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::label>& operands)
	{
		out.emitLabel(operands.label);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
	{
		bool  onoff = (operands.flag != 0);
		auto& spare = cdeftable.substitute(operands.lvid);

		switch (operands.qualifier)
		{
			case 1: // ref
			{
				spare.qualifiers.ref = onoff;
				break;
			}
			case 2: // const
			{
				spare.qualifiers.constant = onoff;
				break;
			}
			default:
				ICE() << "unknown qualifier constant " << operands.qualifier;
		}
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jmp>& opc)
	{
		out.emit<IR::ISA::Op::jmp>() = opc;
	}

	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jz>& opc)
	{
		out.emit<IR::ISA::Op::jz>() = opc;
	}

	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jnz>& opc)
	{
		out.emit<IR::ISA::Op::jnz>() = opc;
	}

	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::comment>& opc)
	{
		// keep the comments in debug
		if (Yuni::debugmode)
			out.emitComment(currentSequence.stringrefs[opc.text]);
	}




	template<IR::ISA::Op O>
	void SequenceBuilder::visit(const IR::ISA::Operand<O>& operands)
	{
		complainOperand(IR::Instruction::fromOpcode(operands));
	}


	bool SequenceBuilder::readAndInstanciate(uint32_t offset)
	{
		currentSequence.each(*this, offset);
		return success;
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany




namespace Nany
{

	bool Isolate::instanciate(Logs::Report report, const AnyString& entrypoint)
	{
		// lock the isolate
		MutexLocker locker{mutex};

		// try to find the entrypoint
		Atom* entrypointAtom = nullptr;
		{
			bool canContinue = true;
			classdefTable.atoms.root.eachChild(entrypoint, [&](Atom& child) -> bool
			{
				if (entrypointAtom != nullptr)
				{
					canContinue = false;
					report.error() << "failed to instanciate '" << entrypoint << "': multiple entry points found";
					return false;
				}
				entrypointAtom = &child;
				return true;
			});

			if (not canContinue)
				return false;
		}

		if (unlikely(nullptr == entrypointAtom))
		{
			report.ICE() << "failed to instanciate '" << entrypoint << "()': function not found";
			return false;
		}

		if (unlikely(entrypointAtom->type != Atom::Type::funcdef))
		{
			report.ICE() << "failed to instanciate '" << entrypoint << "': the atom is not a function";
			return false;
		}

		// parameters for the signature
		decltype(FuncOverloadMatch::result.params)  params;
		decltype(FuncOverloadMatch::result.params)  tmplparams;
		Logs::Message::Ptr newReport;

		ClassdefTableView cdeftblView{classdefTable};

		Pass::Instanciate::InstanciateData info{newReport, *entrypointAtom, cdeftblView, context, params, tmplparams};
		auto* sequence = Pass::Instanciate::InstanciateAtom(info);
		report.appendEntry(newReport);
		return (nullptr != sequence);
	}




} // namespace Nany
