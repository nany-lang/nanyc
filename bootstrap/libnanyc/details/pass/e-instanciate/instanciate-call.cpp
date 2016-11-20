#include "instanciate.h"
#include "instanciate-atom.h"
#include "instanciate-error.h"
#include "overloaded-func-call-resolution.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{

	namespace {


	template<class P, class O>
	bool fetchPushedParameters(const P& pushedparams, O& overloadMatch, const AtomStackFrame& frame)
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


	bool emitFuncCall(SequenceBuilder& seq, const ir::ISA::Operand<ir::ISA::Op::call>& operands)
	{
		// alias (to make it local)
		uint32_t lvid = operands.lvid;
		auto& frame = *seq.frame;

		if (not frame.verify(operands.ptr2func) or not frame.verify(lvid))
			return false;

		// assignment should be handled somewhere else
		assert(not frame.lvids(operands.ptr2func).pointerAssignment);

		if (unlikely(frame.lvids(operands.ptr2func).markedAsAny))
			return error() << "can not perform member lookup on 'any'";

		auto& cdeftable = seq.cdeftable;
		// alias to the current atomid
		auto atomid = frame.atomid;
		// classdef of the function to call
		auto& cdefFuncToCall = cdeftable.classdef(CLID{atomid, operands.ptr2func});
		// any atom already attached ?
		auto* atom = cdefFuncToCall.atom;

		// all pushed parameters
		decltype(FuncOverloadMatch::result.params) params;
		// all pushed template parameters
		decltype(FuncOverloadMatch::result.params) tmplparams;

		// whatever the result of this func, 'lvid' is a returned value
		frame.lvids(lvid).origin.returnedValue = true;

		// preparing the overload matcher
		auto& overloadMatch = seq.overloadMatch;
		overloadMatch.clear();
		overloadMatch.input.rettype.push_back(CLID{atomid, lvid});

		// inserting the 'self' variable if a referer exists
		LVID referer = frame.lvids(operands.ptr2func).referer;
		if (referer != 0)
		{
			// double indirection - TODO find a beter way
			//  %y = %x."foo"
			//  %z = resolve %y."^()" - due to intermediate representation of func call
			//
			// In some other cases, only one is needed, especially for functors
			if (not frame.lvids(referer).singleHopForReferer)
				referer = frame.lvids(referer).referer;
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
					if (atom and atom->isCtor())
						seq.pushCapturedVarsAsParameters(*selfAtom);
				}
			}
		}
		else
		{
			// implicit parameter 'self' ?
			// no referer ('a.foo', 'a' would be the referer), but we may have 'self' as implici parameter
			if (frame.atom.isClassMember())
			{
				Atom* callParent;
				if (nullptr == atom)
				{
					// the solutions should all have the same parent
					auto it = frame.partiallyResolved.find(cdefFuncToCall.clid);
					if (it != frame.partiallyResolved.end())
					{
						auto& solutions = it->second;
						callParent = (not solutions.empty()) ? solutions[0].get().parent : nullptr;
					}
					else
						callParent = nullptr;
				}
				else
					callParent = atom->parent;

				if (callParent and callParent == frame.atom.parent) // method from the same class
				{
					// 0: invalid, 1: return type, 2: first parameter (self here)
					overloadMatch.input.params.indexed.emplace_back(CLID{frame.atomid, 2});
				}
			}
		}

		// overloadMatch: retrieve / append all pushed parameters (indexed, named, generic types)
		if (not fetchPushedParameters(seq.pushedparams, overloadMatch, frame))
			return false;

		if (nullptr == atom)
		{
			// the func to call is not really known. there may have several solutions
			// so we have to investigate which one could be the best one

			// retrieving the list of all available solutions
			// (from previous call to opcode 'resolve')
			auto it = frame.partiallyResolved.find(cdefFuncToCall.clid);
			if (unlikely(it == frame.partiallyResolved.end()))
				return seq.complainOperand(ir::Instruction::fromOpcode(operands), "no solution available");

			auto& solutions = it->second;
			if (unlikely(solutions.empty()))
				return seq.complainOperand(ir::Instruction::fromOpcode(operands), "no solution available");

			OverloadedFuncCallResolver resolver{&seq, seq.report, overloadMatch, cdeftable, seq.build};
			if (unlikely(not resolver.resolve(solutions)))
				return complain::multipleOverloads(operands.ptr2func, solutions, resolver);

			assert(resolver.atom != nullptr and "atom not properly initialized");
			assert(resolver.params != nullptr);
			assert(resolver.tmplparams != nullptr);
			atom = resolver.atom;
			params.swap(*(resolver.params));
			tmplparams.swap(*(resolver.tmplparams));
		}
		else
		{
			assert(0 == frame.partiallyResolved.count(cdefFuncToCall.clid));

			// no overload, the func to call is known
			if (unlikely(not atom->isFunction()))
				return seq.complainOperand(ir::Instruction::fromOpcode(operands), "a functor is required for func call");

			// try to validate the func call
			// (no error reporting, since no overload is present)
			if (unlikely(TypeCheck::Match::none == overloadMatch.validate(*atom)))
				return seq.complainCannotCall(*atom, overloadMatch);

			// get new parameters
			params.swap(overloadMatch.result.params);
			tmplparams.swap(overloadMatch.result.tmplparams);
		}

		if (atom->builtinalias.empty()) // normal func call
		{
			Logs::Message::Ptr subreport;
			InstanciateData info{subreport, *atom, cdeftable, seq.build, params, tmplparams};
			if (not seq.doInstanciateAtomFunc(subreport, info, lvid)) // instanciate the called func
				return false;

			if (seq.canGenerateCode())
			{
				for (auto& element: params) // push all parameters
					seq.out->emitPush(element.clid.lvid());
				seq.out->emitCall(lvid, atom->atomid, info.instanceid);
			}
			return true;
		}
		else
		{
			// not a normal func call, calling builtin intrinsic
			if (unlikely(not tmplparams.empty()))
				return (ice() << "invalid template parameters for builtinalias");
			if (unlikely(seq.pushedparams.func.indexed.size() != params.size()))
				return (error() << "builtin alias not allowed for methods");
			// update each lvid, since they may have been changed (via implicit ctors)
			for (uint32_t i = 0; i != params.size(); ++i)
			{
				assert(seq.pushedparams.func.indexed[i].lvid == params[i].clid.lvid());
				seq.pushedparams.func.indexed[i].lvid = params[i].clid.lvid();
			}
			seq.shortcircuit.compareTo = atom->parameters.shortcircuitValue;
			bool builtinok =
				(Tribool::Value::yes == seq.instanciateBuiltinIntrinsic(atom->builtinalias, lvid));
			if (unlikely(not builtinok))
				frame.invalidate(lvid);
			return builtinok;
		}
	}


	//! Generate short circuit jumps
	bool generateShortCircuitInstrs(SequenceBuilder& seq, uint32_t retlvid)
	{
		assert(seq.canGenerateCode());
		// insert some code after the computation of the first argument but before
		// the computation of the second one to achieve minimal evaluation

		// during the transformation of the AST into opcodes, a label has been
		// generated (shortcircuit.label) and a local variable as well in the same
		// (with the exact value shortcircuit.label + 1), followed by a few 'nop' opcodes
		uint32_t label = seq.shortcircuit.label;
		uint32_t lvid  = label + 1;
		uint32_t lvidBoolResult = label + 2;

		auto& frame = *seq.frame;
		auto& out = seq.out;

		if (not frame.verify(lvid) or not frame.verify(lvidBoolResult))
			return false;

		uint32_t offset = frame.lvids(lvid).offsetDeclOut;
		if (unlikely(not (offset > 0 and offset < out->opcodeCount())))
			return (ice() << "invalid opcode offset for generating shortcircuit");

		// checking if the referenced offset is really a stackalloc
		assert(out->at(offset).opcodes[0] == static_cast<uint32_t>(ir::ISA::Op::stackalloc));

		// lvid of the first parameter
		uint32_t lvidvalue = seq.pushedparams.func.indexed[0].lvid;
		auto& cdef = seq.cdeftable.classdef(CLID{frame.atomid, lvidvalue});
		if (cdef.kind != nyt_bool)
		{
			auto* atom = seq.cdeftable.findClassdefAtom(cdef);
			if (unlikely(seq.cdeftable.atoms().core.object[nyt_bool] != atom))
				return (error() << "boolean expected");

			uint32_t newlvid = out->at<ir::ISA::Op::stackalloc>(offset).lvid;
			++offset;
			assert(out->at(offset).opcodes[0] == static_cast<uint32_t>(ir::ISA::Op::nop));

			auto& fieldget  = out->at<ir::ISA::Op::fieldget>(offset);
			fieldget.opcode = static_cast<uint32_t>(ir::ISA::Op::fieldget);
			fieldget.lvid   = newlvid;
			fieldget.self   = lvidvalue;
			fieldget.var    = 0;
			lvidvalue = newlvid;
		}

		// go to the next nop (can be first one if the parameter was __bool)
		++offset;
		assert(out->at(offset).opcodes[0] == static_cast<uint32_t>(ir::ISA::Op::nop));

		if (not seq.shortcircuit.compareTo) // if true then
		{
			auto& condjmp  = out->at<ir::ISA::Op::jz>(offset);
			condjmp.opcode = static_cast<uint32_t>(ir::ISA::Op::jz); // promotion
			condjmp.lvid   = lvidvalue;
			condjmp.result = retlvid; // func return
			condjmp.label  = label;
		}
		else // if false then
		{
			auto& condjmp  = out->at<ir::ISA::Op::jnz>(offset);
			condjmp.opcode = static_cast<uint32_t>(ir::ISA::Op::jnz); // promotion
			condjmp.lvid   = lvidvalue;
			condjmp.result = retlvid; // func return
			condjmp.label  = label;
		}
		seq.shortcircuit.label = 0; // reset
		return true;
	}


	bool emitPropsetCall(SequenceBuilder& seq, const ir::ISA::Operand<ir::ISA::Op::call>& operands)
	{
		if (unlikely(seq.pushedparams.func.indexed.size() != 1))
			return (ice() << "calling a property setter with more than one value");
		if (unlikely(not seq.pushedparams.func.named.empty()))
			return (ice() << "calling property setter with named parameters");
		if (unlikely(not seq.pushedparams.gentypes.empty()))
			return (ice() << "calling a property setter with generic types parameters");

		auto& frame = *seq.frame;
		// the current context atom
		uint32_t atomid = frame.atomid;
		// result of the func call
		uint32_t lvid = operands.lvid;
		// report for instanciation
		Logs::Message::Ptr subreport;
		// all pushed parameters
		decltype(FuncOverloadMatch::result.params) params;
		// all pushed template parameters
		decltype(FuncOverloadMatch::result.params) tmplparams;

		auto& cdeffunc = seq.cdeftable.classdef(CLID{atomid, operands.ptr2func});
		auto* atom = seq.cdeftable.findClassdefAtom(cdeffunc);
		if (unlikely(!atom))
			return (ice() << "invalid atom for property setter");
		if (unlikely(not atom->isPropertySet()))
			return (ice() << "atom is not a property setter");

		// self, if any
		uint32_t self = frame.lvids(operands.ptr2func).propsetCallSelf;
		if (self == (uint32_t) -1) // unwanted value just for indicating propset call
		{
			// no self parameter, actually here, no self provided by the code
			self = 0;
			// an implicit 'self' may be required
			if (frame.atom.parent and atom->parent and frame.atom.parent->isClass())
			{
				if (atom->parent->atomid == frame.atom.parent->atomid)
					self = 2; // 'self' parameter of the current function
			}
		}

		// the new value for the property
		auto propvalue = seq.pushedparams.func.indexed[0];

		// preparing the overload matcher
		auto& overloadMatch = seq.overloadMatch;
		overloadMatch.clear();
		overloadMatch.input.rettype.push_back(CLID{atomid, lvid});
		if (self != 0)
			overloadMatch.input.params.indexed.emplace_back(CLID{atomid, self});
		overloadMatch.input.params.indexed.emplace_back(CLID{atomid, propvalue.lvid});

		// try to validate the func call
		if (unlikely(TypeCheck::Match::none == overloadMatch.validate(*atom)))
			return seq.complainCannotCall(*atom, overloadMatch);

		// get new parameters
		params.swap(overloadMatch.result.params);
		tmplparams.swap(overloadMatch.result.tmplparams);


		InstanciateData info{subreport, *atom, seq.cdeftable, seq.build, params, tmplparams};
		if (not seq.doInstanciateAtomFunc(subreport, info, lvid))
			return false;

		if (seq.canGenerateCode())
		{
			for (auto& param: params)
				seq.out->emitPush(param.clid.lvid());
			seq.out->emitCall(lvid, atom->atomid, info.instanceid);
		}
		return true;
	}


	} // anonymous namespace




	void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::call>& operands)
	{
		// A 'call' can represent several language features.
		// after AST transformation, assignments are method calls
		// ('a = b' have been transformed into 'a.=(b)'). However this is not
		// a real func call and it is intercepted to be handled differently
		//
		// It can also be a call to a property setter, which is also a func
		// but the input parameters can be slighty different

		// the result is no longer a synthetic object
		frame->lvids(operands.lvid).synthetic = false;
		// resul of the operation
		bool callSuccess;

		if (0 == frame->lvids(operands.ptr2func).propsetCallSelf)
		{
			// normal function call, or assignment
			callSuccess = ((not frame->lvids(operands.ptr2func).pointerAssignment)
				? emitFuncCall(*this, operands)
				: instanciateAssignment(operands));

			if (shortcircuit.label != 0)
			{
				if (callSuccess and canGenerateCode())
					callSuccess = generateShortCircuitInstrs(*this, operands.lvid);
			}
		}
		else
		{
			// property setter
			callSuccess = emitPropsetCall(*this, operands);
		}

		// always remove pushed parameters, whatever the result is
		pushedparams.clear();

		if (unlikely(not callSuccess))
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
} // namespace ny
