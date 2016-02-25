#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::ret>& operands)
	{
		// current frame
		auto& frame = atomStack.back();
		if (unlikely(frame.atom.type != Atom::Type::funcdef)) // just in case
			return (void)(error() << "return values are only accepted in functions");

		// note: the 'return' opcode accepts null value in lvid input (as 'return void')
		bool hasReturnValue = (operands.lvid != 0);

		if (hasReturnValue)
		{
			if (not frame.verify(operands.lvid))
				return;
			if (unlikely(frame.lvids[operands.lvid].markedAsAny))
				return (void)(error() << "return: can not perform member lookup on 'any'");
			if (unlikely(frame.lvids[operands.lvid].synthetic))
				return (void)(error() << "cannot return a synthetic object");
		}

		// the clid to remember for the return value
		CLID returnCLIDForReference;
		// clid from the prototype of the func
		auto& expectedReturnClid = frame.atom.returnType.clid;
		// even with a return value, it can be void
		bool isVoid = (not hasReturnValue);
		// strategy for the returned value
		auto strategy = TypeCheck::Match::strictEqual;

		if (not expectedReturnClid.isVoid())
		{
			auto& expectedCdef = cdeftable.classdefFollowClassMember(expectedReturnClid);
			if (not expectedCdef.isVoid())
			{
				if (unlikely(not hasReturnValue))
					return (void)(error() << "return-statement with no value");

				// classdef of the returned expression
				auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, operands.lvid});
				if (unlikely(cdef.isAny() and nullptr == cdeftable.findClassdefAtom(cdef)))
				{
					ICE() << "invalid 'any' type in return-statement (from "
						<< CLID{frame.atomid, operands.lvid} << ')';
					return;
				}

				// determining if the expression returned matched the return type of the current func
				{
					auto similarity = TypeCheck::isSimilarTo(cdeftable, nullptr, cdef, expectedCdef);
					switch (similarity)
					{
						case TypeCheck::Match::strictEqual:
						{
							returnCLIDForReference = cdef.clid;
							break;
						}
						case TypeCheck::Match::equal:
						{
							auto err = (error() << "implicit conversions are not allowed in 'return'");
							auto hint = err.hint();
							cdef.print((hint << "got '"), cdeftable);
							expectedCdef.print((hint << "', expected '"), cdeftable);
							hint << '\'';
							return;
						}
						case TypeCheck::Match::none:
						{
							auto err = (error() << "type mismatch in 'return' statement");
							auto hint = err.hint();
							cdef.print((hint << "got '"), cdeftable);
							expectedCdef.print((hint << "', expected '"), cdeftable);
							hint << '\'';
							return;
						}
					}
				}

				// the return expr matches the return type requested by the source code
				// if this type is any, checking for previous return types
				if (expectedCdef.isAny() and not frame.returnValues.empty())
				{
					auto& marker = frame.returnValues.front();
					auto& cdefPreviousReturn = cdeftable.classdef(marker.clid);
					auto similarity = TypeCheck::isSimilarTo(cdeftable, nullptr, cdef, cdefPreviousReturn);
					switch (similarity)
					{
						case TypeCheck::Match::strictEqual:
						{
							break; // nice ! they share the same exact type !
						}
						case TypeCheck::Match::equal:
						{
							auto err = (error() << "implicit conversions are not allowed in 'return'\n");
							auto hint = err.hint();
							cdef.print((hint << "got '"), cdeftable);
							cdefPreviousReturn.print((hint << "', expected '"), cdeftable);
							hint << '\'';
							hint.origins().location.pos.line   = marker.line;
							hint.origins().location.pos.offset = marker.offset;
							return;
						}
						case TypeCheck::Match::none:
						{
							auto err = (error() << "multiple return value with different types\n");
							auto hint = err.hint();
							cdef.print((hint << "got '"), cdeftable);
							cdefPreviousReturn.print((hint << "', expected '"), cdeftable);
							hint << '\'';
							hint.origins().location.pos.line   = marker.line;
							hint.origins().location.pos.offset = marker.offset;
							return;
						}
					}
				}
			}
			else
				isVoid = true;
		}
		else
			isVoid = true;


		// the func is returning 'void'
		if (isVoid and hasReturnValue)
		{
			auto& cdef = cdeftable.classdef(CLID{frame.atomid, operands.lvid});
			if (unlikely(not cdef.isVoid()))
				return (void)(error() << "return-statement with a value, in function returning 'void'");
		}

		auto& spare = cdeftable.substitute(1);
		if (not returnCLIDForReference.isVoid())
		{
			assert(not isVoid);
			auto& cdefRetDeduced = cdeftable.classdefFollowClassMember(returnCLIDForReference);

			spare.import(cdefRetDeduced);
			if (not cdefRetDeduced.isBuiltinOrVoid())
				spare.mutateToAtom(cdeftable.findClassdefAtom(cdefRetDeduced));
		}
		else
		{
			assert(isVoid);
			spare.mutateToVoid();
		}


		// the return valus is accepted
		frame.returnValues.emplace_back(ReturnValueMarker {
			returnCLIDForReference,
			currentLine, currentOffset
		});

		if (canGenerateCode())
		{
			uint32_t retlvid;
			switch (strategy)
			{
				case TypeCheck::Match::strictEqual:
				case TypeCheck::Match::equal:
				{
					// acquire the returned object (before releasing variable of the current scope)
					retlvid = operands.lvid;
					if (not isVoid)
						tryToAcquireObject(retlvid, spare);
					break;
				}
				case TypeCheck::Match::none:
				{
					assert(false and "invalid return value strategy");
					ICE() << "invalid return value";
					return;
				}
			}

			// release all variables declared in the function
			releaseScopedVariables(0);
			// the return value
			out.emitReturn(retlvid);
		}
	}







} // namespace Instanciate
} // namespace Pass
} // namespace Nany
