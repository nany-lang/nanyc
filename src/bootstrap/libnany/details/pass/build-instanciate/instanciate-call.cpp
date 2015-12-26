#include "instanciate.h"
#include "details/atom/classdef.h"
#include "details/atom/atom.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	inline bool ProgramBuilder::instanciateFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		// the current frame
		auto& frame = atomStack.back();
		// assignment should be handled somewhere else
		assert(not frame.lvids[operands.ptr2func].isAssignment);

		if (not frame.verify(operands.ptr2func) or not frame.verify(operands.lvid))
			return false;

		if (unlikely(frame.lvids[operands.ptr2func].markedAsAny))
			return error() << "can not perform member lookup on 'any'";

		// alias to the current atomid
		auto atomid = frame.atomid;
		// classdef of the function to call
		auto& cdefFuncToCall = cdeftable.classdef(CLID{atomid, operands.ptr2func});
		// any atom already attached ?
		auto* atom = cdefFuncToCall.atom;

		// all pushed parameters
		decltype(FuncOverloadMatch::result.params) params;


		// preparing the overload matcher
		overloadMatch.clear();
		overloadMatch.input.rettype.push_back(CLID{atomid, operands.lvid});


		// inserting the 'self' variable if a referer exists
		assert(operands.ptr2func < frame.lvids.size());
		LVID referer = frame.lvids[operands.ptr2func].referer;
		if (referer != 0)
		{
			// double indirection - TODO find a beter way
			//  %y = %x."foo"
			//  %z = resolve %y."^()" - due to intermediate representation of func call
			assert(referer < frame.lvids.size());
			referer = frame.lvids[referer].referer;
		}

		if (referer != 0)
		{
			auto& cdefReferer = cdeftable.classdef(CLID{atomid, referer});
			const Atom* selfAtom = cdeftable.findClassdefAtom(cdefReferer);
			if (selfAtom and selfAtom->type != Atom::Type::namespacedef)
				overloadMatch.input.indexedParams.emplace_back(CLID{atomid, referer});
		}
		else
		{
			// no referer (a.foo, a would be the referer), but we may have 'self' as implici paramete)
			if (frame.atom.isFunction() and frame.atom.isClassMember())
			{
				Atom* callParent = atom;
				if (nullptr == callParent)
				{
					auto& solutions = frame.resolvePerCLID[cdefFuncToCall.clid];
					if (not solutions.empty())
						callParent = &(solutions[0].get());
				}
				else
					callParent = atom->parent;

				if (callParent and callParent == frame.atom.parent) // method from the same class
				{
					// 0: invalid, 1: return type, 2: first parameter
					overloadMatch.input.indexedParams.emplace_back(CLID{frame.atomid, 2});
				}
			}
		}


		// parameters
		for (auto indxparm: lastPushedIndexedParameters)
		{
			if (not frame.verify(indxparm.lvid))
				return false;
			overloadMatch.input.indexedParams.emplace_back(CLID{atomid, indxparm.lvid});
		}
		// named parameters
		for (auto nmparm: lastPushedNamedParameters)
		{
			if (not frame.verify(nmparm.lvid))
				return false;
			overloadMatch.input.namedParams.emplace_back(std::make_pair(nmparm.name, CLID{atomid, nmparm.lvid}));
		}


		if (nullptr == atom)
		{
			// the func to call is not really known. there may have several solutions
			// so we have to investigate which one could be the best one

			// retrieving the list of all available solutions
			// (from previous call to opcode 'resolve')
			auto& solutions = frame.resolvePerCLID[cdefFuncToCall.clid];
			if (unlikely(solutions.empty()))
				return complainOperand(reinterpret_cast<const IR::Instruction&>(operands), "no solution available");


			OverloadedFuncCallResolver resolver{report, overloadMatch, cdeftable, intrinsics};
			if (unlikely(not resolver.resolve(solutions)))
				return complainMultipleOverloads(operands.ptr2func, solutions, resolver);

			assert(resolver.atom != nullptr and "atom not properly initialized");
			assert(resolver.params != nullptr);
			atom = resolver.atom;
			params.swap(*(resolver.params));
		}
		else
		{
			assert(frame.resolvePerCLID[cdefFuncToCall.clid].empty());

			// no overload, the func to call is known
			if (unlikely(not atom->isFunction()))
				return complainOperand(reinterpret_cast<const IR::Instruction&>(operands), "a functor is required for func call");

			// any error can be directly reported (since no overload is present)
			overloadMatch.canGenerateReport = false;
			// try to validate the func call
			if (unlikely(Match::none == overloadMatch.validate(*atom)))
			{
				// no match, re-launching the process with error-enabled logging
				overloadMatch.canGenerateReport = true;
				auto err = (error() << "cannot call '" << cdeftable.keyword(*atom) << ' ');
				atom->appendCaption(err.data().message, cdeftable);
				err << '\'';
				overloadMatch.report = std::ref(err);
				overloadMatch.validate(*atom);
				return false;
			}

			// get new parameters
			params.swap(overloadMatch.result.params);
		}

		// instanciate the called func
		Logs::Message::Ptr subreport;
		InstanciateData info{subreport, *atom, cdeftable, intrinsics, params};
		bool instok = doInstanciateAtomFunc(subreport, info, operands.lvid);
		if (unlikely(not instok))
			return false;

		// opcodes
		if (canGenerateCode())
		{
			// push all parameters
			for (auto& element: params)
				out.emitPush(element.clid.lvid());

			out.emitCall(operands.lvid, atom->atomid, info.instanceid);

			auto& lvidinfo = frame.lvids[operands.lvid];
			lvidinfo.origin.returnedValue = true;

			// the function is responsible for acquiring the returned object
			// however we must release it
			if (canBeAcquired(operands.lvid))
				lvidinfo.autorelease = true;
		}
		return true;
	}


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		// after AST transformation, assignments are method calls
		// ('a = b' have been transformed into 'a.=(b)'). However this is not
		// a real func call and it is intercepted to be handled differently
		auto& frame = atomStack.back();
		bool checkpoint = ((not frame.lvids[operands.ptr2func].isAssignment)
			? instanciateFuncCall(operands)
			: instanciateAssignment(operands));

		if (unlikely(not checkpoint))
		{
			frame.invalidate(operands.lvid);
			success = false;
		}

		// always remove pushed parameters, whatever the result
		lastPushedNamedParameters.clear();
		lastPushedIndexedParameters.clear();
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
