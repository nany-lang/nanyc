#include "func-overload-match.h"
#include "details/atom/atom.h"
#include "details/atom/vardef.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"
#include "instanciate.h"
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	FuncOverloadMatch::FuncOverloadMatch(SequenceBuilder* seq)
		: seq(seq)
	{
		input.rettype.reserve(1); // currently, only 1 return value is allowed
		input.params.indexed.reserve(Config::maxPushedParameters);
		input.params.named.reserve(Config::maxPushedParameters);
		result.params.reserve(Config::maxPushedParameters);
	}


	void FuncOverloadMatch::clear()
	{
		input.rettype.clear();
		input.params.indexed.clear();
		input.params.named.clear();
		input.tmplparams.indexed.clear();
		input.tmplparams.named.clear();

		result.params.clear();
		result.tmplparams.clear();
		result.funcToCall = nullptr;
	}


	void FuncOverloadMatch::printInputParameters(String& out) const
	{
		auto& table = seq->cdeftable;
		auto paramprinter = [&](const auto& paramlist)
		{
			if (not paramlist.indexed.empty())
			{
				table.classdef(paramlist.indexed.front()).print(out, table, false);

				for (auto it = paramlist.indexed.begin() + 1; it != paramlist.indexed.end(); ++it)
				{
					out.write(", ", 2);
					table.classdef(*it).print(out, table, false);
				}
			}

			if (not paramlist.named.empty())
			{
				if (not paramlist.indexed.empty())
					out.write(", ", 2);

				(out << paramlist.named.front().first).write(": ", 2);
				table.classdef(paramlist.named.front().second).print(out, table, false);

				for (auto it = paramlist.named.begin() + 1; it != paramlist.named.end(); ++it)
				{
					out.write(", ", 2);
					(out << it->first).write(": ", 2);
					table.classdef(it->second).print(out, table, false);
				}
			}
		};

		if (not input.tmplparams.indexed.empty() or not input.tmplparams.named.empty())
		{
			out << "<:";
			paramprinter(input.tmplparams);
			out << ":>";
		}

		out << '(';
		paramprinter(input.params);
		out << ')';
	}


	void FuncOverloadMatch::complainParamTypeMismatch(bool isGenType, const Classdef& cdef, const Atom& atom, uint32_t i, const Classdef& paramdef)
	{
		assert(report);
		auto& table = seq->cdeftable;
		auto h = report->hint();
		switch (i)
		{
			case 0:  h << "1st"; break;
			case 1:  h << "2nd"; break;
			case 2:  h << "3rd"; break;
			default: h << (i + 1) << "th";
		}

		if (isGenType)
			h << " generic";
		h << " parameter, got '";
		cdef.print(h.message.message, table, false);
		if (debugmode)
			h << ' ' << cdef.clid;

		h << "', expected '";
		paramdef.print(h.message.message, table, false);
		if (debugmode)
			h << ' ' << paramdef.clid;
		h << '\'';

		if (debugmode)
		{
			h.hint() << "failed to push"
				<< (isGenType ? " generic value " : " value ")
				<< cdef.clid << " to " << (CLID{atom.atomid,0})
				<< ":'" << atom.name() << "' parameter index " << (i + 1);

			String inputs;
			printInputParameters(inputs);
			h.hint() << ">> <call> " << inputs;
		}
	}


	template<bool withErrorReporting, bool isTmpl>
	inline TypeCheck::Match FuncOverloadMatch::pushParameter(Atom& atom, uint32_t index, const CLID& clid)
	{
		// force reset
		auto& table = seq->cdeftable;
		auto& cdef  = table.classdef(clid);
		auto& resultinfo = (not isTmpl) ? result.params[index] : result.tmplparams[index];
		resultinfo.clid = clid;
		resultinfo.cdef = &cdef;

		// type checking
		auto& paramdef = (not isTmpl)
			? table.classdef(atom.parameters.vardef(index).clid)
			: table.classdef(atom.tmplparams.vardef(index).clid);

		// checking the parameter type
		resultinfo.strategy = (not cdef.isAny())
			? TypeCheck::isSimilarTo(*seq, cdef, paramdef, pAllowImplicit)
			: TypeCheck::Match::none;

		if (resultinfo.strategy == TypeCheck::Match::none)
		{
			if (withErrorReporting)
				complainParamTypeMismatch(isTmpl, cdef, atom, index, paramdef);
		}
		return resultinfo.strategy;
	}


	template<bool withErrorReporting>
	inline TypeCheck::Match FuncOverloadMatch::validateAtom(Atom& atom, bool allowImplicit)
	{
		assert(atom.isFunction() or atom.isClass());

		// some reset
		result.params.clear();
		result.tmplparams.clear();
		result.funcToCall = &atom;

		auto& table = seq->cdeftable;

		// trivial check, too many parameters for this overload
		if (unlikely(atom.parameters.size() < (uint32_t) input.params.indexed.size()))
		{
			if (withErrorReporting)
			{
				assert(report);
				// do not take into consideration the 'self' parameter for error reporting
				uint32_t selfidx = static_cast<uint32_t>(atom.isClassMember() and atom.isFunction());
				report->hint() << "too many parameters. Got "
					<< (input.params.indexed.size() - selfidx)
					<< ", expected: " << (atom.parameters.size() - selfidx);
			}
			return TypeCheck::Match::none;
		}
		// trivial check, too many template parameters for this overload
		if (unlikely(atom.tmplparams.size() < (uint32_t) input.tmplparams.indexed.size()))
		{
			if (withErrorReporting)
			{
				assert(report);
				report->hint() << "too many generic type parameters. Got "
					<< input.tmplparams.indexed.size()
					<< ", expected: " << atom.tmplparams.size();
			}
			return TypeCheck::Match::none;
		}

		pAllowImplicit = allowImplicit;
		bool perfectMatch /*= false*/;

		// checking input.params
		{
			perfectMatch = true;
			result.params.resize(atom.parameters.size());
			result.tmplparams.resize(atom.tmplparams.size());

			// trying to resolve indexed template parameters
			for (uint32_t i = 0; i != (uint32_t) input.tmplparams.indexed.size(); ++i)
			{
				switch (pushParameter<withErrorReporting, true>(atom, i, input.tmplparams.indexed[i]))
				{
					case TypeCheck::Match::equal:       perfectMatch = false; break;
					case TypeCheck::Match::strictEqual: break;
					case TypeCheck::Match::none:        return TypeCheck::Match::none;
				}
			}

			if (unlikely(not input.tmplparams.named.empty()))
			{
				if (withErrorReporting)
					report->error() << "named generic type parameters not implemented yet";
				return TypeCheck::Match::none;
			}


			// trying to resolve indexed parameters (if they match)
			for (uint32_t i = 0; i != (uint32_t) input.params.indexed.size(); ++i)
			{
				switch (pushParameter<withErrorReporting, false>(atom, i, input.params.indexed[i]))
				{
					case TypeCheck::Match::equal:       perfectMatch = false; break;
					case TypeCheck::Match::strictEqual: break;
					case TypeCheck::Match::none:        return TypeCheck::Match::none;
				}
			}

			// try to resolve named-parameters
			if (not input.params.named.empty())
			{
				uint32_t offset = (uint32_t) input.params.indexed.size();
				for (auto& pair: input.params.named)
				{
					uint32_t index = atom.parameters.findByName(pair.first, offset);
					// the named parameter is not present after indexed parameters
					if (unlikely(index == static_cast<uint32_t>(-1)))
					{
						//report.trace() << "named parameter '" << pair.first
						//	<< "' not found after started from index " << offset << " in " << (CLID{atom.atomid,0});
						if (withErrorReporting)
							report->hint() << "named parameter '" << pair.first << "' not found after indexed parameters";
						return TypeCheck::Match::none;
					}

					switch (pushParameter<withErrorReporting, false>(atom, index, pair.second))
					{
						case TypeCheck::Match::equal:       perfectMatch = false; break;
						case TypeCheck::Match::strictEqual: break;
						case TypeCheck::Match::none:        return TypeCheck::Match::none;
					}
				}
			}

			// check for missing default values for generic types
			for (size_t i = input.tmplparams.indexed.size(); i < result.tmplparams.size(); ++i)
			{
				if (not result.tmplparams[i].cdef or result.tmplparams[i].cdef->clid.isVoid()) // undefined - TODO: default values
				{
					if (withErrorReporting)
					{
						auto ix = i;
						if (not atom.isClassMember())
							++ix;
						auto h = (report->hint());
						h << atom.caption(table);
						h << ": no type provided for the generic type parameter '";
						h << atom.tmplparams.name((uint32_t) i) << '\'';
					}
					return TypeCheck::Match::none;
				}
			}

			// check for missing default values
			for (size_t i = input.params.indexed.size(); i < result.params.size(); ++i)
			{
				if (not result.params[i].cdef or result.params[i].cdef->clid.isVoid()) // undefined - TODO: default values
				{
					if (withErrorReporting)
					{
						auto ix = i;
						if (not atom.isClassMember())
							++ix;
						auto h = (report->hint());
						h << atom.caption(table) << ": no value provided for the parameter '";
						h << atom.parameters.name((uint32_t) i) << '\'';
					}
					return TypeCheck::Match::none;
				}
			}
		}

		// checking return type
		switch (input.rettype.size())
		{
			case 1:
			{
				auto& wantedRettype = table.classdef(input.rettype[0]);
				// the value is really used, let's continue
				if (wantedRettype.isBuiltinOrVoid() or wantedRettype.hasConstraints())
				{
					auto& atomRettype = table.classdef(atom.returnType.clid);
					if (TypeCheck::Match::none == TypeCheck::isSimilarTo(*seq, wantedRettype, atomRettype, pAllowImplicit))
					{
						if (withErrorReporting)
						{
							auto err = (report->hint() << "returned type does not match, got '");
							if (debugmode)
								err << atomRettype.clid;
							atomRettype.print(err.message.message, table, false);
							err << "', expected '";

							if (debugmode)
								err << wantedRettype.clid;
							wantedRettype.print(err.message.message, table, false);
							err << '\'';
						}
						return TypeCheck::Match::none;
					}
				}
				break;
			}
			case 0:
			{
				// no return value, the return type MUST be void
				if (not atom.returnType.clid.isVoid())
				{
					auto& atomRettype = table.classdef(atom.returnType.clid);
					if (atomRettype.isVoid())
						break;

					// The returned type can be any, which can be void then
					if (atomRettype.kind == nyt_any and !table.findClassdefAtom(atomRettype))
						break;

					if (withErrorReporting)
					{
						auto err = (report->hint() << "returned type does not match, got '");
						if (debugmode)
							err << atomRettype.clid;
						atomRettype.print(err.message.message, table, false);
						err << "', expected 'void'";
					}
					return TypeCheck::Match::none;
				}
				break;
			}
			default:
			{
				assert(false and "not implemented");
				return TypeCheck::Match::none;
			}
		}
		return perfectMatch ? TypeCheck::Match::strictEqual : TypeCheck::Match::equal;
	}




	TypeCheck::Match FuncOverloadMatch::validate(Atom& atom, bool allowImplicit)
	{
		return validateAtom<false>(atom, allowImplicit);
	}

	TypeCheck::Match FuncOverloadMatch::validateWithErrReport(Atom& atom, bool allowImplicit)
	{
		assert(report != nullptr);
		auto match = validateAtom<true>(atom, allowImplicit);
		report = nullptr;
		return match;
	}



} // namespace Instanciate
} // namespace Pass
} // namespace Nany
