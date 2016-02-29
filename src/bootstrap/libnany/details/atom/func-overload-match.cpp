#include "func-overload-match.h"
#include "details/atom/atom.h"
#include "details/atom/vardef.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"
#include "libnany-config.h"
#include <iostream>

using namespace Yuni;





namespace Nany
{

	FuncOverloadMatch::FuncOverloadMatch(Logs::Report report, const ClassdefTableView& table)
		: report(std::ref(report))
		, table(table)
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
		if (not input.params.indexed.empty())
		{
			table.classdef(input.params.indexed.front()).print(out, table, false);

			for (auto it = input.params.indexed.begin() + 1; it != input.params.indexed.end(); ++it)
			{
				out.write(", ", 2);
				table.classdef(*it).print(out, table, false);
			}
		}

		if (not input.params.named.empty())
		{
			if (not input.params.indexed.empty())
				out.write(", ", 2);

			(out << input.params.named.front().first).write(": ", 2);
			table.classdef(input.params.named.front().second).print(out, table, false);

			for (auto it = input.params.named.begin() + 1; it != input.params.named.end(); ++it)
			{
				out.write(", ", 2);
				(out << it->first).write(": ", 2);
				table.classdef(it->second).print(out, table, false);
			}
		}
	}


	template<bool isTmpl>
	inline TypeCheck::Match
	FuncOverloadMatch::pushParameter(Atom& atom, yuint32 index, const CLID& clid)
	{
		if (isTmpl)
		std::cout << " :: pushing parameter " << index << std::endl;
		// force reset
		auto& cdef = table.classdef(clid);
		auto& resultinfo = (not isTmpl) ? result.params[index] : result.tmplparams[index];
		resultinfo.clid = clid;
		resultinfo.cdef = &cdef;

		// type checking
		auto& paramdef = (not isTmpl)
			? table.classdef(atom.parameters.vardef(index).clid)
			: table.classdef(atom.tmplparams.vardef(index).clid);

		resultinfo.strategy = TypeCheck::isSimilarTo(table, nullptr, cdef, paramdef, pAllowImplicit);
		if (unlikely(canGenerateReport) and resultinfo.strategy == TypeCheck::Match::none)
		{
			report.get().trace() << "failed to push"
				<< (isTmpl ? " generic value " : " value ")
				<< cdef.clid << " to parameter "
				<< (CLID{atom.atomid,0})
				<< " '" << atom.name << "' index " << index;

			auto err = report.get().hint();
			switch (index)
			{
				case 0: err << "1st"; break;
				case 1: err << "2nd"; break;
				case 2: err << "3rd"; break;
				default: err << (index + 1) << "th";
			}
			if (isTmpl)
				err << " generic";
			err << " parameter, got '";
			cdef.print(err.message.message, table, false);
			if (debugmode)
				err << ' ' << cdef.clid;

			err << "', expected '";
			paramdef.print(err.message.message, table, false);
			if (debugmode)
				err << ' ' << paramdef.clid;
			err << '\'';
		}
		return resultinfo.strategy;
	}


	TypeCheck::Match FuncOverloadMatch::validate(Atom& atom, bool allowImplicit)
	{
		// some reset
		result.params.clear();
		result.tmplparams.clear();
		result.funcToCall = &atom;

		if (unlikely(not atom.isFunction()))
			return TypeCheck::Match::none;

		// trivial check, too many parameters for this overload
		if (atom.parameters.size() < (uint32_t) input.params.indexed.size())
		{
			if (unlikely(canGenerateReport))
			{
				// do not take into consideration the 'self' parameter for error reporting
				uint32_t selfidx = static_cast<uint32_t>(atom.isClassMember() and atom.isFunction());
				report.get().hint() << "too many parameters. Got "
					<< (input.params.indexed.size() - selfidx)
					<< ", expected: " << (atom.parameters.size() - selfidx);
			}
			return TypeCheck::Match::none;
		}
		// trivial check, too many template parameters for this overload
		if (atom.tmplparams.size() < (uint32_t) input.tmplparams.indexed.size())
		{
			if (unlikely(canGenerateReport))
			{
				report.get().hint() << "too many generic type parameters. Got "
					<< input.tmplparams.indexed.size()
					<< ", expected: " << atom.tmplparams.size();
			}
			return TypeCheck::Match::none;
		}

		pAllowImplicit = allowImplicit;
		bool perfectMatch /*= false*/;

		// checking input.params
		// determine whether there is at least one parameter, in the atom or pushed
		if (hasAtLeastOneParameter(atom))
		{
			perfectMatch = true;
			result.params.resize(atom.parameters.size());
			result.tmplparams.resize(atom.tmplparams.size());

			// trying to resolve indexed template parameters
			for (uint32_t i = 0; i != (uint32_t) input.tmplparams.indexed.size(); ++i)
			{
				switch (pushParameter<true>(atom, i, input.tmplparams.indexed[i]))
				{
					case TypeCheck::Match::equal:       perfectMatch = false; break;
					case TypeCheck::Match::strictEqual: break;
					case TypeCheck::Match::none:        return TypeCheck::Match::none;
				}
			}

			// trying to resolve indexed parameters (if they match)
			for (uint32_t i = 0; i != (uint32_t) input.params.indexed.size(); ++i)
			{
				switch (pushParameter<false>(atom, i, input.params.indexed[i]))
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
					if (index == static_cast<uint32_t>(-1))
					{
						//report.trace() << "named parameter '" << pair.first
						//	<< "' not found after started from index " << offset << " in " << (CLID{atom.atomid,0});
						if (unlikely(canGenerateReport))
							report.get().hint() << "named parameter '" << pair.first << "' not found after indexed parameters";
						return TypeCheck::Match::none;
					}

					switch (pushParameter<false>(atom, index, pair.second))
					{
						case TypeCheck::Match::equal:       perfectMatch = false; break;
						case TypeCheck::Match::strictEqual: break;
						case TypeCheck::Match::none:        return TypeCheck::Match::none;
					}
				}
			}

			if (not input.tmplparams.named.empty())
			{
				if (unlikely(canGenerateReport))
					report.get().error() << "named generic type parameters not implemented yet";
				return TypeCheck::Match::none;
			}

			// check for missing default values for generic types
			for (size_t i = input.tmplparams.indexed.size(); i < result.tmplparams.size(); ++i)
			{
				if (nullptr == result.tmplparams[i].cdef or result.tmplparams[i].cdef->clid.isVoid()) // undefined - TODO: default values
				{
					if (unlikely(canGenerateReport))
					{
						auto ix = i;
						if (not atom.isClassMember())
							++ix;
						auto hint = (report.get().hint());
						hint << atom.caption(table) << ": no type provided for the ";
						hint << "generic type parameter '";
						hint << atom.tmplparams.name((uint32_t) i) << '\'';
					}
					return TypeCheck::Match::none;
				}
			}

			// check for missing default values
			for (size_t i = input.params.indexed.size(); i < result.params.size(); ++i)
			{
				if (nullptr == result.params[i].cdef or result.params[i].cdef->clid.isVoid()) // undefined - TODO: default values
				{
					if (unlikely(canGenerateReport))
					{
						auto ix = i;
						if (not atom.isClassMember())
							++ix;
						auto hint = (report.get().hint());
						hint << atom.caption(table) << ": no value provided for the parameter '";
						hint << atom.parameters.name((uint32_t) i) << '\'';
					}
					return TypeCheck::Match::none;
				}
			}
		}
		else
		{
			// do not allow perfect matching without parameters
			perfectMatch = false;
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
					if (TypeCheck::Match::none == TypeCheck::isSimilarTo(table, nullptr, wantedRettype, atomRettype, pAllowImplicit))
					{
						if (unlikely(canGenerateReport))
						{
							auto err = report.get().hint() << "returned type does not match, got '";
							atomRettype.print(err.message.message, table, false);
							err << "', expected '";
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
				// return type must be void
				if (not atom.returnType.clid.isVoid())
				{
					auto& atomRettype = table.classdef(atom.returnType.clid);
					if (unlikely(not atomRettype.isVoid()))
					{
						if (unlikely(canGenerateReport))
						{
							auto err = report.get().hint() << "returned type does not match, got '";
							atomRettype.print(err.message.message, table, false);
							err << "', expected 'void'";
						}
						return TypeCheck::Match::none;
					}
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




} // namespace Nany
