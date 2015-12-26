#include "classdef-table.h"



namespace Nany
{

	inline Match ClassdefTable::isAtomSimilarTo(const CTarget* target, const Atom& atom, const Atom& to) const
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

				for (uint p = 0; p != atom.parameters.size(); ++p)
				{
					auto& atomParam = atom.parameters[p];
					auto& toParam   = to.parameters[p];
					if (atomParam.first != toParam.first)
						return Match::none;

					auto& cdefAtom = classdef(atomParam.second.clid);
					auto& cdefTo   = classdef(toParam.second.clid);

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
					auto& cdefAtom = classdef(atom.returnType.clid);
					auto& cdefTo   = classdef(to.returnType.clid);
					if (Match::none == isSimilarTo(target, cdefAtom, cdefTo, false))
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
						if (Match::none != isAtomSimilarTo(target, child, toChild))
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



	namespace // anonymous
	{

		static inline Match BuiltinCompareMatrix(nytype_t a, nytype_t b)
		{
			switch (b)
			{
				case nyt_i16:
				case nyt_u16:
				{
					if (a == nyt_i8)
						return Match::equal;
					if (a == nyt_u8)
						return Match::equal;
					break;
				}
				case nyt_i32:
				case nyt_u32:
				{
					if (a == nyt_i8 or a == nyt_i16)
						return Match::equal;
					if (a == nyt_u8 or a == nyt_u16)
						return Match::equal;
					break;
				}
				case nyt_i64:
				case nyt_u64:
				{
					if (a == nyt_i8 or a == nyt_i16 or a == nyt_i32)
						return Match::equal;
					if (a == nyt_u8 or a == nyt_u16 or a == nyt_u32)
						return Match::equal;
					break;
				}
				case nyt_f64:
				{
					if (a == nyt_f32)
						return Match::equal;
					break;
				}
				default:
					break;
			}

			return Match::none;
		}


	} // anonymous namespace


	Match ClassdefTable::isSimilarTo(const CTarget* target, const Classdef& cdef, const Classdef& to, bool allowImplicit) const
	{
		// identity
		// (note: comparing only the address of 'cdef' and 'to' is not good enough
		//   since symlink may exist in the classdef table)
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
		{
			if (cdef.kind == to.kind)
				return Match::strictEqual;
			return BuiltinCompareMatrix(cdef.kind, to.kind);
		}


		const Atom* toAtom = findClassdefAtom(to);
		if (unlikely(toAtom == nullptr)) // type not resolved
			return Match::none;


		auto similarity = Match::none;
		if (cdef.hasAtom())
		{
			similarity = isAtomSimilarTo(target, *cdef.atom, *toAtom);
			if (similarity == Match::none)
				return Match::none;
		}

		// follow-ups
		for (auto& clid: cdef.followup.extends)
		{
			auto extendSimilarity = isSimilarTo(target, classdef(clid), to, allowImplicit);
			if (Match::none == extendSimilarity)
				return Match::none;
			if (similarity == Match::none)
				similarity = extendSimilarity;
		}

		return similarity;
	}




} // namespace Nany
