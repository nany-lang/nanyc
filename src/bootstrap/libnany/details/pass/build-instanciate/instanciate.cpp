#include "instanciate.h"
#include "details/context/isolate.h"
#include <memory>
#include "details/reporting/report.h"
#include "details/atom/func-overload-match.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "details/context/context.h"
#include "libnany-traces.h"

using namespace Yuni;






namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	InstanciateData::InstanciateData(Logs::Message::Ptr& report, Atom& atom, ClassdefTableView& cdeftable,
		nycontext_t& context, decltype(FuncOverloadMatch::result.params)& params)
		: report(report)
		, atom(atom)
		, cdeftable(cdeftable)
		, context(context)
		, params(params)
	{
		returnType.mutateToAny();
	}


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
		// reduce memory allocations
		atomStack.reserve(4); // arbitrary
		lastPushedNamedParameters.reserve(32); // arbitrary
		lastPushedIndexedParameters.reserve(32);
		multipleResults.reserve(8); // arbitrary value
	}


	SequenceBuilder::~SequenceBuilder()
	{
		if (Config::Traces::printClassdefTable)
			printClassdefTable();
	}


	void SequenceBuilder::printClassdefTable()
	{
		while (not atomStack.empty())
		{
			printClassdefTable(report.subgroup(), atomStack.back());
			atomStack.pop_back();
		}
	}


	void SequenceBuilder::releaseScopedVariables(int scope, bool forget)
	{
		if (unlikely(atomStack.empty()))
			return;
		if (not forget and (not canGenerateCode()))
			return;

		auto& frame = atomStack.back();

		// unref in the reverse order
		auto i = static_cast<uint32_t>(frame.lvids.size());

		if (canGenerateCode())
		{
			while (i-- != 0)
			{
				auto& clcvr = frame.lvids[i];
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
							complainUnusedVariable(frame, i);
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
				auto& clcvr = frame.lvids[i];
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
		assert(not atomStack.empty());
		assert(count > 0);

		auto& operands = out.at<IR::ISA::Op::stacksize>(lastOpcodeStacksizeOffset);
		uint32_t startOffset = operands.add;
		auto& frame = atomStack.back();

		if (count == 1)
		{
			frame.resizeRegisterCount((++operands.add), cdeftable);

			if (canGenerateCode())
			{
				auto& lvidinfo = frame.lvids[startOffset];
				lvidinfo.scope = frame.scope;
				lvidinfo.offsetDeclOut = out.opcodeCount();
				out.emitStackalloc(startOffset, nyt_any);
			}
			else
				frame.lvids[startOffset].scope = frame.scope;
		}
		else
		{
			operands.add += count;
			frame.resizeRegisterCount(operands.add, cdeftable);

			if (canGenerateCode())
			{
				for (uint32_t i = 0; i != count; ++i)
				{
					auto& lvidinfo = frame.lvids[startOffset + i];
					lvidinfo.scope = frame.scope;
					lvidinfo.offsetDeclOut = out.opcodeCount();
					out.emitStackalloc(startOffset + i, nyt_any);
				}
			}
			else
			{
				for (uint32_t i = 0; i != count; ++i)
					frame.lvids[startOffset + i].scope = frame.scope;
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
			}
			else
			{
				entry << "    " << clid << ": !!INVALID CLID";
			}
		}
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
		Logs::Message::Ptr newReport;

		ClassdefTableView cdeftblView{classdefTable};

		Pass::Instanciate::InstanciateData info{newReport, *entrypointAtom, cdeftblView, context, params};
		auto* sequence = Pass::Instanciate::InstanciateAtom(info);
		report.appendEntry(newReport);
		return (nullptr != sequence);
	}




} // namespace Nany
