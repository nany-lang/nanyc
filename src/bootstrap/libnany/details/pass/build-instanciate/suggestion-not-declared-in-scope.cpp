#include "instanciate.h"
#include <yuni/string.h>
#include <yuni/core/math.h>
#include <vector>
#include <set>
#include "details/fwd.h"
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

		static uint LevenshteinDistance(const AnyString& source, const AnyString& target)
		{
			if (source.empty())
				return target.size();

			if (target.empty())
				return source.size();

			// our matrix
			std::vector<std::vector<uint>>  matrix(source.size() + 1);

			// Size the vectors in the 2.nd dimension. Unfortunately C++ doesn't
			// allow for allocation on declaration of 2.nd dimension of vec of vec
			for (uint i = 0; i <= source.size(); ++i)
				matrix[i].resize(target.size() + 1);

			for (uint i = 0; i <= source.size(); ++i)
				matrix[i][0] = i;

			for (uint j = 0; j <= target.size(); ++j)
				matrix[0][j] = j;

			for (uint i = 1; i <= source.size(); ++i)
			{
				auto s_i = source[i - 1];

				// Step 4
				for (uint j = 1; j <= target.size(); ++j)
				{
					auto t_j = target[j - 1];

					uint cost = (s_i == t_j) ? 0 : 1;

					// Step 6
					uint above = matrix[i - 1][j];
					uint left  = matrix[i][j - 1];
					uint diag  = matrix[i - 1][j - 1];

					uint cell = std::min(above + 1, std::min(left + 1, diag + cost));

					if (i > 2 and j > 2)
					{
						uint trans = matrix[i - 2][j - 2] + 1;

						if (source[i - 2] != t_j)
							++trans;
						if (s_i != target[j - 2])
							++trans;
						if (cell > trans)
							cell = trans;
					}

					matrix[i][j] = cell;
				}
			}

			return matrix[source.size()][target.size()];
		}



		static inline bool stringsAreCloseEnough(uint& note, const AnyString& a, const AnyString& b)
		{
			note = LevenshteinDistance(a, b) + (uint) a.size();
			note += (uint) Math::Abs((ssize_t) a.size() - (ssize_t) b.size());
			return (note <= 8);
		}


	} // anonymous namespace




	bool SequenceBuilder::complainUnknownIdentifier(const Atom* self, const Atom& atom, const AnyString& name)
	{
		assert(not name.empty());
		if (unlikely(name.empty())) // should never happen
			return (error() << "invalid empty identifier name");

		if (self and self->flags(Atom::Flags::error)) // error already reported
			return false;

		auto err = error();
		bool unknownIsOperator = (name.first() == '^');
		if (unknownIsOperator)
		{
			AnyString rname{name.c_str() + 1, name.size() - 1};
			if (self)
			{
				err << "'operator " << rname << "' is not declared in '";
				err << cdeftable.keyword(*self) << ' ';
				self->retrieveCaption(err.data().message, cdeftable);
				err << '\'';
			}
			else
				err << "'operator " << rname << "' is not declared in this scope";
		}
		else
		{
			if (self)
			{
				err << '\'' << name << "' is not declared in '";
				err << cdeftable.keyword(*self) << ' ';
				self->retrieveCaption(err.data().message, cdeftable);
				err << '\'';
			}
			else
				err << '\'' << name << "' is not declared in this scope";
		}


		// trying local variables first
		if (self == nullptr and not unknownIsOperator)
		{
			uint note;

			// reverse order, to get the nearest first
			uint i = (uint) frame->lvids.size();
			while (i-- > 0)
			{
				auto& crlcvr = frame->lvids[i];

				// only take non-empty names and avoid the hidden parameter 'self'
				if (crlcvr.userDefinedName.empty() or crlcvr.userDefinedName == "self")
					continue;

				if (not stringsAreCloseEnough(note, name, crlcvr.userDefinedName))
					continue;

				auto suggest = (err.suggest() << "var " << crlcvr.userDefinedName);
				auto* varAtom = cdeftable.findClassdefAtom(cdeftable.classdef(CLID{frame->atomid, i}));
				if (varAtom)
				{
					suggest << ": ";
					suggest << cdeftable.keyword(*varAtom) << ' ';
					varAtom->retrieveCaption(suggest.data().message, cdeftable);
				}

				suggest.origins().location.pos.line   = crlcvr.file.line;
				suggest.origins().location.pos.offset = crlcvr.file.offset;
				suggest.origins().location.filename   = crlcvr.file.url;
			}
		}


		auto* parentAtom = (self) ? self : atom.parent;
		if (parentAtom)
		{
			std::map<size_t, std::vector<std::reference_wrapper<const Atom>>> dict;

			if (not unknownIsOperator)
			{
				// not an operator, can be anything
				parentAtom->eachChild([&](const Atom& child) -> bool
				{
					if (&child != &atom and (not child.isOperator()))
					{
						uint note;
						if (stringsAreCloseEnough(note, name, child.name))
							dict[note].emplace_back(std::cref(child));
					}
					return true;
				});
			}
			else
			{
				// invalid operator, suggesting only operator with exactly the same name
				parentAtom->eachChild([&](const Atom& child) -> bool
				{
					if (&child != &atom and child.name == name)
					{
						// arbitrary mark based on the number of parameters
						// for pseudo ordering
						dict[child.parameters.size()].emplace_back(std::cref(child));
					}
					return true;
				});
			}

			for (auto& pair: dict)
			{
				for (auto& candidateptr: pair.second)
				{
					auto& candidate = candidateptr.get();
					if (candidate.flags(Atom::Flags::suggestInReport))
					{
						auto suggest = (err.suggest() << cdeftable.keyword(candidate) << ' ');
						candidate.retrieveCaption(suggest.data().message, cdeftable);

						suggest.origins().location.pos.line   = candidate.origin.line;
						suggest.origins().location.pos.offset = candidate.origin.offset;
						suggest.origins().location.filename   = candidate.origin.filename;
					}
				}
			}
		}

		return false;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
