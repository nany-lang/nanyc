#include "instanciate.h"
#include "instanciate-atom.h"
#include "overloaded-func-call-resolution.h"
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	namespace // anonymous
	{

		template<class P, class O>
		static inline bool fetchPushedParameters(const P& pushedparams, O& overloadMatch, const AtomStackFrame& frame)
		{
			uint32_t atomid = frame.atomid;

			// parameters
			for (auto indxparm: pushedparams.func.indexed)
			{
				if (not frame.verify(indxparm.lvid))
					return false;
				overloadMatch.input.params.indexed.emplace_back(CLID{atomid, indxparm.lvid});
			}
			// named parameters
			for (auto nmparm: pushedparams.func.named)
			{
				if (not frame.verify(nmparm.lvid))
					return false;
				overloadMatch.input.params.named.emplace_back(std::make_pair(nmparm.name, CLID{atomid, nmparm.lvid}));
			}

			// template parameters
			for (auto indxparm: pushedparams.gentypes.indexed)
			{
				if (not frame.verify(indxparm.lvid))
					return false;
				overloadMatch.input.tmplparams.indexed.emplace_back(CLID{atomid, indxparm.lvid});
			}
			// named template parameters
			for (auto nmparm: pushedparams.gentypes.named)
			{
				if (not frame.verify(nmparm.lvid))
					return false;
				overloadMatch.input.tmplparams.named.emplace_back(std::make_pair(nmparm.name, CLID{atomid, nmparm.lvid}));
			}
			return true;
		}


		static inline bool atomIsCtor(const Atom* atom)
		{
			if (atom and atom->isCtor())
				return true;
			return false;
		}


	} // anonymous namespace




	inline bool SequenceBuilder::emitFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		// alias (to make it local)
		uint32_t lvid = operands.lvid;

		if (not frame->verify(operands.ptr2func) or not frame->verify(lvid))
			return false;

		// assignment should be handled somewhere else
		assert(not frame->lvids[operands.ptr2func].pointerAssignment);

		if (unlikely(frame->lvids[operands.ptr2func].markedAsAny))
			return error() << "can not perform member lookup on 'any'";

		// alias to the current atomid
		auto atomid = frame->atomid;
		// classdef of the function to call
		auto& cdefFuncToCall = cdeftable.classdef(CLID{atomid, operands.ptr2func});
		// any atom already attached ?
		auto* atom = cdefFuncToCall.atom;

		// all pushed parameters
		decltype(FuncOverloadMatch::result.params) params;
		// all pushed template parameters
		decltype(FuncOverloadMatch::result.params) tmplparams;

		// whatever the result of this func, 'lvid' is a returned value
		frame->lvids[lvid].origin.returnedValue = true;

		// preparing the overload matcher
		overloadMatch.clear();
		overloadMatch.input.rettype.push_back(CLID{atomid, lvid});


		// inserting the 'self' variable if a referer exists
		LVID referer = frame->lvids[operands.ptr2func].referer;
		if (referer != 0)
		{
			// double indirection - TODO find a beter way
			//  %y = %x."foo"
			//  %z = resolve %y."^()" - due to intermediate representation of func call
			//
			// In some other cases, only one is needed, especially for functors
			assert(referer < frame->lvids.size());
			if (not frame->lvids[referer].singleHopForReferer)
				referer = frame->lvids[referer].referer;
		}

		if (referer != 0)
		{
			auto& cdefReferer = cdeftable.classdef(CLID{atomid, referer});
			const Atom* selfAtom = cdeftable.findClassdefAtom(cdefReferer);

			if (selfAtom and selfAtom->type != Atom::Type::namespacedef)
			{
				overloadMatch.input.params.indexed.emplace_back(CLID{atomid, referer});

				// get the captured variables and push them as named parameters
				if (selfAtom->flags(Atom::Flags::pushCapturedVariables))
				{
					if (atomIsCtor(atom))
						pushCapturedVarsAsParameters(*selfAtom);
				}
			}
		}
		else
		{
			// implicit parameter 'self' ?
			// no referer ('a.foo', 'a' would be the referer), but we may have 'self' as implici parameter
			if (frame->atom.isClassMember())
			{
				Atom* callParent;
				if (nullptr == atom)
				{
					// the solutions should all have the same parent
					auto it = frame->partiallyResolved.find(cdefFuncToCall.clid);
					if (it != frame->partiallyResolved.end())
					{
						auto& solutions = it->second;
						callParent = (not solutions.empty()) ? solutions[0].get().parent : nullptr;
					}
					else
						callParent = nullptr;
				}
				else
					callParent = atom->parent;

				if (callParent and callParent == frame->atom.parent) // method from the same class
				{
					// 0: invalid, 1: return type, 2: first parameter
					overloadMatch.input.params.indexed.emplace_back(CLID{frame->atomid, 2});
				}
			}
		}


		// Get all parameters (indexed, named, generic types...)
		if (not fetchPushedParameters(pushedparams, overloadMatch, *frame))
			return false;


		if (nullptr == atom)
		{
			// the func to call is not really known. there may have several solutions
			// so we have to investigate which one could be the best one

			// retrieving the list of all available solutions
			// (from previous call to opcode 'resolve')
			auto it = frame->partiallyResolved.find(cdefFuncToCall.clid);
			if (unlikely(it == frame->partiallyResolved.end()))
				return complainOperand(IR::Instruction::fromOpcode(operands), "no solution available");

			auto& solutions = it->second;
			if (unlikely(solutions.empty()))
				return complainOperand(IR::Instruction::fromOpcode(operands), "no solution available");


			OverloadedFuncCallResolver resolver{this, report, overloadMatch, cdeftable, build};
			if (unlikely(not resolver.resolve(solutions)))
				return complainMultipleOverloads(operands.ptr2func, solutions, resolver);

			assert(resolver.atom != nullptr and "atom not properly initialized");
			assert(resolver.params != nullptr);
			assert(resolver.tmplparams != nullptr);
			atom = resolver.atom;
			params.swap(*(resolver.params));
			tmplparams.swap(*(resolver.tmplparams));
		}
		else
		{
			assert(0 == frame->partiallyResolved.count(cdefFuncToCall.clid));

			// no overload, the func to call is known
			if (unlikely(not atom->isFunction()))
				return complainOperand(IR::Instruction::fromOpcode(operands), "a functor is required for func call");

			// any error can be directly reported (since no overload is present)
			overloadMatch.canGenerateReport = false;
			// try to validate the func call
			if (unlikely(TypeCheck::Match::none == overloadMatch.validate(*atom)))
			{
				// no match, re-launching the process with error-enabled logging
				overloadMatch.canGenerateReport = true;
				auto err = (error() << "cannot call '" << cdeftable.keyword(*atom) << ' ');
				atom->retrieveCaption(err.data().message, cdeftable);
				err << '\'';
				overloadMatch.report = std::ref(err);
				overloadMatch.validate(*atom);
				return false;
			}

			// get new parameters
			params.swap(overloadMatch.result.params);
			tmplparams.swap(overloadMatch.result.tmplparams);
		}

		if (not atom->builtinalias.empty())
		{
			if (unlikely(not tmplparams.empty()))
				return (ICE() << "invalid template parameters for builtinalias");
			if (unlikely(pushedparams.func.indexed.size() != params.size()))
				return (error() << "builtin alias not allowed for methods");
			// update each lvid, since they may have been changed (via implicit ctors)
			for (uint32_t i = 0; i != params.size(); ++i)
			{
				assert(pushedparams.func.indexed[i].lvid == params[i].clid.lvid());
				pushedparams.func.indexed[i].lvid = params[i].clid.lvid();
			}
			shortcircuit.compareTo = atom->parameters.shortcircuitValue;
			return instanciateBuiltinIntrinsic(atom->builtinalias, lvid);
		}

		// instanciate the called func
		Logs::Message::Ptr subreport;
		InstanciateData info{subreport, *atom, cdeftable, build, params, tmplparams};
		bool instok = doInstanciateAtomFunc(subreport, info, lvid);
		if (unlikely(not instok))
			return false;

		// opcodes
		if (canGenerateCode())
		{
			for (auto& element: params) // push all parameters
				out.emitPush(element.clid.lvid());

			out.emitCall(lvid, atom->atomid, info.instanceid);

			// the function is responsible for acquiring the returned object
			// however we must release it
			if (canBeAcquired(lvid))
				frame->lvids[lvid].autorelease = true;
		}
		return true;
	}


	bool SequenceBuilder::generateShortCircuitInstrs(uint32_t retlvid)
	{
		assert(canGenerateCode());
		// insert some code after the computation of the first argument but before
		// the computation of the second one to achieve minimal evaluation

		// during the transformation of the AST into opcodes, a label has been
		// generated (shortcircuit.label) and a local variable as well in the same
		// (with the exact value shortcircuit.label + 1), followed by a few 'nop' opcodes
		uint32_t label = shortcircuit.label;
		uint32_t lvid  = label + 1;
		uint32_t lvidBoolResult = label + 2;

		if (not frame->verify(lvid) or not frame->verify(lvidBoolResult))
			return false;

		uint32_t offset = frame->lvids[lvid].offsetDeclOut;
		if (unlikely(not (offset > 0 and offset < out.opcodeCount())))
			return (ICE() << "invalid opcode offset for generating shortcircuit");

		// checking if the referenced offset is really a stackalloc
		assert(out.at(offset).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::stackalloc));

		// lvid of the first parameter
		uint32_t lvidvalue = pushedparams.func.indexed[0].lvid;
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, lvidvalue});
		if (cdef.kind != nyt_bool)
		{
			auto* atom = cdeftable.findClassdefAtom(cdef);
			if (unlikely(cdeftable.atoms().core.object[nyt_bool] != atom))
				return (error() << "boolean expected");

			uint32_t newlvid = out.at<IR::ISA::Op::stackalloc>(offset).lvid;
			++offset;
			std::cout << "------ " << out.at(offset).opcodes[0] << '\n';
			std::cout << "------ " << (uint32_t) IR::ISA::Op::label << '\n';
			assert(out.at(offset).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::nop));

			auto& fieldget  = out.at<IR::ISA::Op::fieldget>(offset);
			fieldget.opcode = static_cast<uint32_t>(IR::ISA::Op::fieldget);
			fieldget.lvid   = newlvid;
			fieldget.self   = lvidvalue;
			fieldget.var    = 0;
			lvidvalue = newlvid;
		}

		// go to the next nop (can be first one if the parameter was __bool)
		++offset;
		assert(out.at(offset).opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::nop));

		if (not shortcircuit.compareTo) // if true then
		{
			auto& condjmp  = out.at<IR::ISA::Op::jz>(offset);
			condjmp.opcode = static_cast<uint32_t>(IR::ISA::Op::jz); // promotion
			condjmp.lvid   = lvidvalue;
			condjmp.result = retlvid; // func return
			condjmp.label  = label;
		}
		else // if false then
		{
			auto& condjmp  = out.at<IR::ISA::Op::jnz>(offset);
			condjmp.opcode = static_cast<uint32_t>(IR::ISA::Op::jnz); // promotion
			condjmp.lvid   = lvidvalue;
			condjmp.result = retlvid; // func return
			condjmp.label  = label;
		}

		// reset
		shortcircuit.label = 0;
		return true;
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		// after AST transformation, assignments are method calls
		// ('a = b' have been transformed into 'a.=(b)'). However this is not
		// a real func call and it is intercepted to be handled differently

		// the result is not a synthetic object
		frame->lvids[operands.lvid].synthetic = false;

		bool checkpoint = ((not frame->lvids[operands.ptr2func].pointerAssignment)
			? emitFuncCall(operands)
			: instanciateAssignment(operands));

		if (shortcircuit.label != 0)
		{
			if (checkpoint and canGenerateCode())
				checkpoint = generateShortCircuitInstrs(operands.lvid);
		}

		// always remove pushed parameters, whatever the result
		pushedparams.clear();

		if (unlikely(not checkpoint))
		{
			success = false;
			frame->invalidate(operands.lvid);
			// invalidate metadata from shortcircuit, just in case something bad
			// happened before reaching the code responsible for minimal evaluation
			shortcircuit.label = 0;
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
