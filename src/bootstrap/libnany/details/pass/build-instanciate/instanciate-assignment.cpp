#include "instanciate.h"
#include <iostream>

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	enum class AssignStrategy
	{
		rawregister,
		ref,
		deepcopy,
	};



	bool ProgramBuilder::instanciateAssignment(AtomStackFrame& frame, LVID lhs, LVID rhs, bool canDisposeLHS, bool checktype, bool forceDeepcopy)
	{
		// lhs and rhs can not be null, but they can be identical, to force a clone
		// when required for example
		if (unlikely(lhs == 0 or rhs == 0 or lhs == rhs))
			return (ICE() << "invalid lvid for variable assignment");

		// current atom id
		auto atomid   = frame.atomid;
		// LHS cdef
		auto& cdeflhs = cdeftable.classdefFollowClassMember(CLID{atomid, lhs});
		// RHS cdef
		auto& cdefrhs = cdeftable.classdefFollowClassMember(CLID{atomid, rhs});

		if (checktype)
		{
			auto similarity = cdeftable.isSimilarTo(nullptr, cdeflhs, cdefrhs, false);
			if (unlikely(Match::strictEqual != similarity))
			{
				auto err = (error() << "cannot convert '");
				cdefrhs.print(err.data().message, cdeftable, false);
				err << "' to '";
				cdeflhs.print(err.data().message, cdeftable, false);
				err << "' in initialization";
				return false;
			}
		}

		// type propagation
		if (not canDisposeLHS)
			cdeftable.substitute(lhs).import(cdefrhs);

		if (unlikely(not canGenerateCode()))
			return true;


		// can lhs be acquires ?
		bool lhsCanBeAcquired = canBeAcquired(cdeflhs);

		// deep copy by default
		auto strategy = AssignStrategy::deepcopy;

		if (lhsCanBeAcquired)
		{
			if (not forceDeepcopy)
			{
				// NOTE: the qualifiers from `cdeflhs` are not valid and correspond to nothing
				auto& originalcdef = cdeftable.classdef(CLID{atomid, lhs});
				out.emitComment(originalcdef.print(cdeftable) << originalcdef.clid);

				if (originalcdef.qualifiers.ref)
				{
					strategy = AssignStrategy::ref;
				}
				else
				{
					assert(rhs < frame.lvids.size());
					auto& lvidinfo = frame.lvids[rhs];
					if (lvidinfo.origin.memalloc or lvidinfo.origin.returnedValue)
						strategy = AssignStrategy::ref;
				}
			}
		}
		else
			strategy = AssignStrategy::rawregister;


		auto& lhsLvidinfo = frame.lvids[lhs];
		if (lhsCanBeAcquired)
			lhsLvidinfo.autorelease = true;

		auto& origin = lhsLvidinfo.origin.varMember;
		bool isMemberVariable = (origin.atomid != 0);


		if (debugmode)
		{
			String comment;
			switch (strategy)
			{
				case AssignStrategy::rawregister: comment << "raw copy "; break;
				case AssignStrategy::ref: comment << "assign ref "; break;
				case AssignStrategy::deepcopy: comment << "deep copy "; break;
			}
			comment << "%" << lhs << " = %" << rhs << " aka '";
			cdeflhs.print(comment, cdeftable, false);
			comment << '\'';
			out.emitComment(comment);
		}

		switch (strategy)
		{
			case AssignStrategy::rawregister:
			{
				if (lhs != rhs)
				{
					out.emitStore(lhs, rhs);
					if (isMemberVariable)
						out.emitFieldset(lhs, origin.self, origin.field);
				}
				break;
			}

			case AssignStrategy::ref:
			{
				if (lhs != rhs)
				{
					// acquire first the right value to make sure that all data are alive
					// example: a = a
					out.emitRef(rhs);
					// release the old left value
					if (canDisposeLHS)
					{
						tryUnrefObject(lhs);
						if (isMemberVariable)
							tryUnrefObject(lhs);
					}
					// copy the pointer
					out.emitStore(lhs, rhs);

					if (isMemberVariable)
					{
						out.emitRef(lhs); // re-acquire for the object
						out.emitFieldset(lhs, origin.self, origin.field);
					}
				}
				break;
			}

			case AssignStrategy::deepcopy:
			{
				auto* rhsAtom = cdeftable.findClassdefAtom(cdefrhs);
				if (unlikely(nullptr == rhsAtom))
					return (ICE() << "invalid atom for left-side assignment");

				// 'clone' operator
				if (0 == rhsAtom->classinfo.clone.atomid)
				{
					if (unlikely(not instanciateAtomClassClone(*rhsAtom, lhs, rhs)))
						return false;
				}

				// acquire first the right value to make sure that all data are alive
				// example: a = a
				out.emitRef(rhs);
				// release the old left value
				if (canDisposeLHS)
				{
					tryUnrefObject(lhs);
					if (isMemberVariable)
						tryUnrefObject(lhs);
				}

				// note: do not keep a reference on 'out.at...', since the internal buffer might be reized
				assert(lastOpcodeStacksizeOffset != (uint32_t) -1);
				auto& operands = out.at<IR::ISA::Op::stacksize>(lastOpcodeStacksizeOffset);
				uint32_t lvid = operands.add;
				uint32_t retcall = operands.add + 1;
				operands.add += 2;
				frame.resizeRegisterCount(lvid + 2, cdeftable);

				uint32_t rsizof  = out.emitStackalloc(lvid, nyt_u64);
				out.emitSizeof(rsizof, rhsAtom->atomid);

				// re-allocate some memory
				out.emitMemalloc(lhs, rsizof);
				out.emitRef(lhs);
				assert(lhs < frame.lvids.size());
				frame.lvids[lhs].origin.memalloc = true;

				out.emitStackalloc(retcall, nyt_void);
				// call operator 'clone'
				out.emitPush(lhs); // self
				out.emitPush(rhs); // rhs, the object to copy
				out.emitCall(retcall, rhsAtom->classinfo.clone.atomid, rhsAtom->classinfo.clone.instanceid);

				// release rhs - copy is done
				tryUnrefObject(rhs);

				if (isMemberVariable)
				{
					out.emitRef(lhs);
					out.emitFieldset(lhs, origin.self, origin.field);
				}
				break;
			}
		}

		return true;
	}




	bool ProgramBuilder::instanciateAssignment(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		if (unlikely(lastPushedIndexedParameters.size() != 1))
		{
			ICE() << "assignment: invalid number of pushed parameters";
			return false;
		}
		if (unlikely(not lastPushedNamedParameters.empty()))
		{
			ICE() << "assignment: named parameters are not accepted";
			return false;
		}

		// the current frame
		auto& frame = atomStack.back();

		// -- LHS
		// note: double indirection, since assignment is like method call
		//  %y = %x."="
		//  %z = resolve %y."^()"
		assert(operands.ptr2func < frame.lvids.size());
		LVID lhs = frame.lvids[operands.ptr2func].referer;
		if (likely(0 != lhs))
		{
			assert(lhs < frame.lvids.size());
			lhs  = frame.lvids[lhs].referer;
		}

		// -- RHS
		LVID rhs = lastPushedIndexedParameters[0].lvid;
		lastPushedIndexedParameters.clear();

		return instanciateAssignment(frame, lhs, rhs);
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
