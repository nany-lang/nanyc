#include <yuni/yuni.h>
#include "type-check.h"

using namespace Yuni;




namespace Nany
{
namespace TypeCheck
{

	namespace // anonymous
	{

		static Match isAtomSimilarTo(const ClassdefTable& table, const CTarget* target, const Atom& atom, const Atom& to)
		{
			if (&atom == &to) // identity
				return Match::strictEqual;

			if (atom.type != to.type) // can not be similar to a different type
				return Match::none;


			switch (atom.type)
			{
				case Atom::Type::funcdef:
				{
					if (atom.parameters.size() != to.parameters.size())
						return Match::none;

					for (uint32_t p = 0; p != atom.parameters.size(); ++p)
					{
						auto& atomParam = atom.parameters[p];
						auto& toParam   = to.parameters[p];
						if (atomParam.first != toParam.first)
							return Match::none;

						auto& cdefAtom = table.classdef(atomParam.second.clid);
						auto& cdefTo   = table.classdef(toParam.second.clid);

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
						auto& cdefAtom = table.classdef(atom.returnType.clid);
						auto& cdefTo   = table.classdef(to.returnType.clid);
						if (Match::none == isSimilarTo(table, target, cdefAtom, cdefTo, false))
							return Match::none;
					}
					return Match::equal;
				}

				case Atom::Type::classdef:
				{
					bool found = true;
					atom.eachChild([&](const Atom& child) -> bool
					{
						found = false;
						// try to find a similar atom
						to.eachChild(child.name, [&](const Atom& toChild) -> bool
						{
							if (Match::none != isAtomSimilarTo(table, target, child, toChild))
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

				case Atom::Type::namespacedef:
				case Atom::Type::vardef:
				{
					assert(false and "comparing two namespaces ?"); // Uh ?
					return Match::none;
				}
			}

			assert(false and "some value not handled here...");
			return Match::none;
		}


	} // anonymous namespace





	Match isSimilarTo(const ClassdefTable& table, const CTarget* target, const Classdef& cdef, const Classdef& to,
		bool allowImplicit)
	{
		// identity
		// (note: comparing only the address of 'cdef' and 'to' is not good enough
		//   since symlink may exist in the table.classdef table)
		if (cdef.clid == to.clid)
			return Match::strictEqual;

		// the target accepts anything, even 'const' objects
		if (to.isAny())
			return Match::strictEqual;

		// invalid constness
		if (cdef.qualifiers.constant and (not to.qualifiers.constant))
			return Match::none;

		// same builtin, identity as weel
		if (to.isBuiltinOrVoid() or cdef.isBuiltinOrVoid())
			return (cdef.kind == to.kind) ? Match::strictEqual : Match::none;


		const Atom* toAtom = table.findClassdefAtom(to);
		if (unlikely(toAtom == nullptr)) // type not resolved
			return Match::none;

		auto similarity = Match::none;
		if (cdef.hasAtom())
		{
			similarity = isAtomSimilarTo(table, target, *cdef.atom, *toAtom);
			if (similarity == Match::none)
				return Match::none;
		}

		// follow-ups
		for (auto& clid: cdef.followup.extends)
		{
			auto extendSimilarity = isSimilarTo(table, target, table.classdef(clid), to, allowImplicit);
			if (Match::none == extendSimilarity)
				return Match::none;

			if (similarity == Match::none)
				similarity = extendSimilarity;
		}

		return similarity;
	}





} // namespace TypeCheck
} // namespace Nany
