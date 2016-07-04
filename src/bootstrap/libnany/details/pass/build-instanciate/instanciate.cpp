#include "details/context/build.h"
#include "instanciate.h"
#include "details/reporting/report.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "libnany-traces.h"
#include "instanciate-atom.h"
#include "func-overload-match.h"
#include <memory>

using namespace Yuni;






namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	SequenceBuilder::SequenceBuilder(Logs::Report report, ClassdefTableView& cdeftable, Build& build,
		IR::Sequence& out, IR::Sequence& sequence, SequenceBuilder* parent)
		: cdeftable(cdeftable)
		, build(build)
		, intrinsics(build.intrinsics)
		, out(out)
		, currentSequence(sequence)
		, overloadMatch(this)
		, parent(parent)
		, localErrorHandler(this, &emitReportEntry)
		, localMetadataHandler(this, &retriveReportMetadata)
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
		if (Config::Traces::allTypeDefinitions)
			printClassdefTable();

		auto* frm = frame;
		while (frm)
		{
			auto* previous = frm->previous;
			build.deallocate(frm);
			frm = previous;
		}
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
		assert(count > 0);
		assert(frame != nullptr);
		assert(frame->offsetOpcodeStacksize != (uint32_t) -1);

		auto& operands = out.at<IR::ISA::Op::stacksize>(frame->offsetOpcodeStacksize);
		uint32_t startOffset = operands.add;
		int scope = frame->scope;

		operands.add += count;
		frame->resizeRegisterCount(operands.add, cdeftable);
		assert(startOffset + count <= frame->lvids.size());

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
		return startOffset;
	}


	void SequenceBuilder::printClassdefTable(const AtomStackFrame& currentframe)
	{
		// new entry
		{
			auto entry = trace();
			entry.message.prefix << cdeftable.keyword(currentframe.atom) << ' ';
			currentframe.atom.retrieveCaption(entry.message.prefix, cdeftable);
			entry << " - type matrix, after instanciation - atom " << currentframe.atom.atomid;
		}

		for (uint32_t i = 0; i != currentframe.localVariablesCount(); ++i)
		{
			auto clid = CLID{currentframe.atomid, i};
			auto entry = trace();

			if (cdeftable.hasClassdef(clid) or cdeftable.hasSubstitute(clid))
			{
				cdeftable.printClassdef(entry.message.message, clid, cdeftable.classdef(clid));
				entry.message.message.trimRight();

				if (cdeftable.hasSubstitute(clid))
					entry << " (local replacement)";

				if (currentframe.lvids[i].isConstexpr)
					entry << " (constexpr)";

				if (currentframe.lvids[i].errorReported)
					entry << " [ERROR]";
			}
			else
				entry << "    " << clid << ": !!INVALID CLID";
		}
	}


	void SequenceBuilder::printClassdefTable()
	{
		for (auto* f = frame; f != nullptr; f = f->previous)
			printClassdefTable(*f);
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
				ice() << "invalid inherit value " << operands.inherit;
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
		uint32_t lbl = out.emitLabel(operands.label);
		(void) lbl; // avoid compiler warning for `emitLabel`
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
	{
		if (operands.qualifier < IR::ISA::TypeQualifierCount)
		{
			bool  onoff = (operands.flag != 0);
			auto& spare = cdeftable.substitute(operands.lvid);

			switch (static_cast<IR::ISA::TypeQualifier>(operands.qualifier))
			{
				case IR::ISA::TypeQualifier::ref:
				{
					spare.qualifiers.ref = onoff;
					return;
				}
				case IR::ISA::TypeQualifier::constant:
				{
					spare.qualifiers.constant = onoff;
					return;
				}
			}
		}
		ice() << "unknown qualifier constant " << operands.qualifier;
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


	bool Build::instanciate(const AnyString& entrypoint, const nytype_t* args, uint32_t& atomid, uint32_t& instanceid)
	{
		Nany::Logs::Report report{*messages.get()};

		if (unlikely(args))
		{
			report.error() << "arguments for atom instanciation is not supported yet";
			return false;
		}

		// lock the isolate
		MutexLocker locker{mutex};

		// try to find the entrypoint
		Atom* entrypointAtom = nullptr;
		{
			bool canContinue = true;
			cdeftable.atoms.root.eachChild(entrypoint, [&](Atom& child) -> bool
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
			report.ice() << "failed to instanciate '" << entrypoint << "()': function not found";
			return false;
		}

		if (unlikely(entrypointAtom->type != Atom::Type::funcdef))
		{
			report.ice() << "failed to instanciate '" << entrypoint << "': the atom is not a function";
			return false;
		}

		// parameters for the signature
		decltype(Pass::Instanciate::FuncOverloadMatch::result.params) params;
		decltype(Pass::Instanciate::FuncOverloadMatch::result.params) tmplparams;
		Logs::Message::Ptr newReport;

		ClassdefTableView cdeftblView{cdeftable};

		Pass::Instanciate::InstanciateData info {
			newReport, *entrypointAtom, cdeftblView, *this, params, tmplparams
		};
		bool instanciated = Pass::Instanciate::instanciateAtom(info);
		report.appendEntry(newReport);

		if (Config::Traces::atomTable)
			cdeftable.atoms.root.print(cdeftable);

		if (instanciated)
		{
			atomid = entrypointAtom->atomid;
			instanceid = info.instanceid;
			return true;
		}
		else
		{
			atomid = (uint32_t) -1;
			instanceid = (uint32_t) -1;
			return false;
		}
	}




} // namespace Nany
