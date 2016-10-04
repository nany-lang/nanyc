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
		if (unlikely(frame->atom.type != Atom::Type::funcdef)) // just in case
			return (void)(error() << "return values are only accepted in functions");

		//
		// --- EXPECTED RETURN TYPE
		//
		const Classdef* expectedcdef = nullptr;
		if (not frame->atom.returnType.clid.isVoid())
		{
			expectedcdef = &cdeftable.classdefFollowClassMember(frame->atom.returnType.clid);
			if (expectedcdef->isVoid())
				expectedcdef = nullptr; // void
		}

		//
		// --- USER RETURN VALUE
		//
		// determining the status of the return value
		const Classdef* usercdef = nullptr;
		if (operands.lvid != 0)
		{
			 usercdef = &cdeftable.classdefFollowClassMember(CLID{frame->atomid, operands.lvid});
			 if (usercdef->isVoid())
				 usercdef = nullptr;
		}

		// is the return value 'void' ?
		bool retIsVoid = true;
		// similarty between the two types to detect if an implicit convertion is required
		auto similarity = TypeCheck::Match::strictEqual;

		if (expectedcdef and usercdef)
		{
			// there is a return value !
			retIsVoid = false;

			// determining if the expression returned matched the return type of the current func
			similarity = TypeCheck::isSimilarTo(*this, *usercdef, *expectedcdef);
			switch (similarity)
			{
				case TypeCheck::Match::strictEqual:
				{
					break;
				}
				case TypeCheck::Match::equal:
				{
					if (expectedcdef->isAny()) // accept implicit convertions to 'any'
						break;
					return complainReturnTypeImplicitConv(*expectedcdef, *usercdef);
				}
				case TypeCheck::Match::none:
				{
					return complainReturnTypeMismatch(*expectedcdef, *usercdef);
				}
			}

			// the return expr matches the return type requested by the source code
			// if this type is any, checking for previous return types
			if (expectedcdef->isAny() and not frame->returnValues.empty())
			{
				auto& marker = frame->returnValues.front();
				if (not marker.clid.isVoid())
				{
					auto& cdefPreviousReturn = cdeftable.classdef(marker.clid);
					auto prevsim = TypeCheck::isSimilarTo(*this, *usercdef, cdefPreviousReturn);
					switch (prevsim)
					{
						case TypeCheck::Match::strictEqual:
						{
							break; // nice ! they share the same exact type !
						}
						case TypeCheck::Match::equal:
						{
							return complainReturnTypeImplicitConv(cdefPreviousReturn, *usercdef, marker.line, marker.offset);
						}
						case TypeCheck::Match::none:
						{
							return complainReturnTypeMultiple(cdefPreviousReturn, *usercdef, marker.line, marker.offset);
						}
					}
				}
				else
				{
					// can not be void
					return complainReturnTypeMissing(nullptr, usercdef);
				}
			}
		}
		else
		{
			if (!usercdef and !expectedcdef)
			{
				// both void
			}
			else
			{
				if (expectedcdef and expectedcdef->isAny())
				{
					// accept the following:
					//    func foo: any { return; }
				}
				else
				{
					// one of the values are 'null'
					if (unlikely(usercdef != nullptr or expectedcdef != nullptr))
						return complainReturnTypeMissing(expectedcdef, usercdef);
				}
			}
		}

		auto& spare = cdeftable.substitute(1);
		if (not retIsVoid)
		{
			spare.import(*usercdef);
			if (not usercdef->isBuiltinOrVoid())
				spare.mutateToAtom(cdeftable.findClassdefAtom(*usercdef));
		}
		else
		{
			spare.mutateToVoid();
			spare.qualifiers.clear();
		}

		// the return valus is accepted
		frame->returnValues.emplace_back(ReturnValueMarker {
			(usercdef ? usercdef->clid : CLID{}), currentLine, currentOffset
		});

		if (canGenerateCode())
		{
			switch (similarity)
			{
				case TypeCheck::Match::equal:
				{
					// implicit convertion
					// not currently supported
					// - fallthru
				}
				case TypeCheck::Match::strictEqual:
				{
					if (debugmode)
						out->emitComment("return from func");

					if (not retIsVoid)
					{
						uint32_t retlvid;
						if (spare.qualifiers.ref or spare.isBuiltinOrVoid())
						{
							retlvid = operands.lvid;
						}
						else
						{
							// a copy seems necessary...
							retlvid = operands.tmplvid;
							// trying to copy the value first
							if (unlikely(not frame->verify(retlvid)))
								return frame->invalidate(retlvid);

							auto& spareTmp = cdeftable.substitute(operands.tmplvid);
							spareTmp.import(spare);
							spareTmp.qualifiers.merge(spare.qualifiers);
							frame->lvids[operands.tmplvid].synthetic = false;

							bool r = instanciateAssignment(*frame, retlvid, operands.lvid, false);
							if (unlikely(not r))
								return frame->invalidate(retlvid);
						}

						// acquiring it first before deleting everything
						tryToAcquireObject(retlvid, spare);
						// release all variables declared in the function
						releaseScopedVariables(0 /*all scopes*/);
						// the return value
						out->emitReturn(retlvid, 0);
					}
					else
					{
						releaseScopedVariables(0 /*all scopes*/);
						// the return value
						out->emitReturn();
					}
					break;
				}
				case TypeCheck::Match::none:
				{
				}
			}
		}
	}







} // namespace Instanciate
} // namespace Pass
} // namespace Nany
