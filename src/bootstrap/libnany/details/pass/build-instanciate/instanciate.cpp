#include "instanciate.h"
#include "details/context/isolate.h"
#include <memory>
#include "details/reporting/report.h"
#include "details/atom/func-overload-match.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "libnany-traces.h"

using namespace Yuni;






namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	ProgramBuilder::ProgramBuilder(Logs::Report report, ClassdefTableView& cdeftable, const IntrinsicTable& intrinsics,
		IR::Program& out, IR::Program& program)
		: cdeftable(cdeftable)
		, intrinsics(intrinsics)
		, out(out)
		, currentProgram(program)
		, overloadMatch(report, cdeftable)
		, report(report)
	{
		// reduce memory allocations
		atomStack.reserve(4); // arbitrary
		lastPushedNamedParameters.reserve(32); // arbitrary
		lastPushedIndexedParameters.reserve(32);
		multipleResults.reserve(8); // arbitrary value
	}


	ProgramBuilder::~ProgramBuilder()
	{
		if (Config::Traces::printClassdefTable)
			printClassdefTable();
	}


	void ProgramBuilder::printClassdefTable()
	{
		while (not atomStack.empty())
		{
			printClassdefTable(report.subgroup(), atomStack.back());
			atomStack.pop_back();
		}
	}


	void ProgramBuilder::releaseScopedVariables(int scope, bool forget)
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



	void ProgramBuilder::printClassdefTable(Logs::Report trace, const AtomStackFrame& frame) const
	{
		auto entry = trace.trace();

		entry.message.prefix << cdeftable.keyword(frame.atom) << ' ';
		frame.atom.appendCaption(entry.message.prefix, cdeftable);
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



	bool ProgramBuilder::readAndInstanciate(uint32_t offset)
	{
		currentProgram.each(*this, offset);
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

		Pass::Instanciate::InstanciateData info{newReport, *entrypointAtom, cdeftblView, intrinsics, params};
		auto* program = Pass::Instanciate::InstanciateAtom(info);
		report.appendEntry(newReport);
		return (nullptr != program);
	}




} // namespace Nany
