#include <yuni/yuni.h>
#include "type-check.h"
#include "instanciate.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{
namespace TypeCheck
{

	namespace // anonymous
	{

		static Match isAtomSimilarTo(SequenceBuilder& seq, Atom& atom, Atom& to)
		{
			if (&atom == &to) // identity
				return Match::strictEqual;

			if (atom.type != to.type) // can not be similar to a different type
				return Match::none;

			if (to.isTypeAlias())
			{
				const Classdef* newCdef = nullptr;
				auto& newTo = seq.resolveTypeAlias(to, newCdef);

				// it can't be a builtin since we have an atom
				if (unlikely(!newCdef or newCdef->isBuiltinOrVoid()))
					return Match::none;
				return isAtomSimilarTo(seq, atom, newTo);
			}


			switch (atom.type)
			{
				case Atom::Type::funcdef:
				{
					uint32_t apsize = atom.parameters.size();
					if (apsize != to.parameters.size())
						return Match::none;

					for (uint32_t p = 0; p != apsize; ++p)
					{
						auto& atomParam = atom.parameters[p];
						auto& toParam   = to.parameters[p];
						if (atomParam.first != toParam.first)
							return Match::none;

						auto& cdefAtom = seq.cdeftable.classdef(atomParam.second.clid);
						auto& cdefTo   = seq.cdeftable.classdef(toParam.second.clid);

						if (cdefAtom.atom != cdefTo.atom)
							return Match::none;
						if (cdefAtom.qualifiers != cdefTo.qualifiers)
							return Match::none;
					}

					// checking the return type
					bool hasReturnType = atom.hasReturnType();
					if (hasReturnType != to.hasReturnType())
						return Match::none;

					if (hasReturnType)
					{
						auto& cdefAtom = seq.cdeftable.classdef(atom.returnType.clid);
						auto& cdefTo   = seq.cdeftable.classdef(to.returnType.clid);
						if (Match::none == isSimilarTo(seq, cdefAtom, cdefTo, false))
							return Match::none;
					}
					return Match::equal;
				}

				case Atom::Type::classdef:
				{
					bool found = true;
					atom.eachChild([&](Atom& child) -> bool
					{
						found = false;
						// try to find a similar atom
						to.eachChild(child.name(), [&](Atom& toChild) -> bool
						{
							if (Match::none != isAtomSimilarTo(seq, child, toChild))
							{
								found = true;
								return false;
							}
							return true;
						});
						return found;
					});
					return found ? Match::equal : Match::none;
				}

				case Atom::Type::typealias:
				{
					assert(false and "type comparison - with typedef - implementation missing");
					break;
				}
				case Atom::Type::namespacedef:
				case Atom::Type::vardef:
				case Atom::Type::unit:
				{
					assert(false and "invalid type comparison");
					break;
				}
			}
			return Match::none;
		}


	} // anonymous namespace





	Match isSimilarTo(SequenceBuilder& seq, const Classdef& from, const Classdef& to, bool allowImplicit)
	{
		// identity
		// (note: comparing only the address of 'from' and 'to' is not good enough
		//   since symlink may exist in the table.classdef table)
		if (from.clid == to.clid)
			return Match::strictEqual;

		// constness
		//if (from.qualifiers.constant and (not to.qualifiers.constant))
		//	return Match::none;

		// the target accepts anything
		if (to.isAny())
			return Match::equal;

		// same builtin, identity as weel
		if (to.isBuiltinOrVoid() or from.isBuiltinOrVoid())
			return (from.kind == to.kind) ? Match::strictEqual : Match::none;


		Atom* toAtom = seq.cdeftable.findClassdefAtom(to);
		if (unlikely(toAtom == nullptr)) // type not resolved
			return Match::none;


		auto similarity = Match::none;

		if (not toAtom->isTypeAlias())
		{
			do
			{
				if (from.hasAtom())
				{
					similarity = isAtomSimilarTo(seq, *from.atom, *toAtom);
					if (similarity == Match::none)
						break;
				}

				// follow-ups
				for (auto& clid: from.followup.extends)
				{
					auto extendSimilarity = isSimilarTo(seq, seq.cdeftable.classdef(clid), to, allowImplicit);
					if (Match::none == extendSimilarity)
						return Match::none;

					if (similarity == Match::none)
						similarity = extendSimilarity;
				}
			} while (false);
		}
		else
		{
			const Classdef* newCdef = nullptr;
			/*auto& newTo =*/ seq.resolveTypeAlias(*toAtom, newCdef);
			if (newCdef)
				similarity = isSimilarTo(seq, from, (*newCdef), allowImplicit);
		}
		return similarity;
	}





} // namespace TypeCheck
} // namespace Instanciate
} // namespace Pass
} // namespace ny
