#include "func-overload-match.h"
#include "details/atom/atom.h"
#include "details/atom/vardef.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"
#include "libnany-config.h"

using namespace Yuni;





namespace Nany
{

	FuncOverloadMatch::FuncOverloadMatch(Logs::Report report, const ClassdefTableView& table)
		: report(std::ref(report))
		, table(table)
	{
		input.rettype.reserve(1); // currently, only 1 return value is allowed
		input.indexedParams.reserve(Config::maxPushedParameters);
		input.namedParams.reserve(Config::maxPushedParameters);
		result.params.reserve(Config::maxPushedParameters);
	}


	void FuncOverloadMatch::clear()
	{
		input.rettype.clear();
		input.indexedParams.clear();
		input.namedParams.clear();

		result.params.clear();
		result.funcToCall = nullptr;
	}


	void FuncOverloadMatch::printInputParameters(String& out) const
	{
		if (not input.indexedParams.empty())
		{
			table.classdef(input.indexedParams.front()).print(out, table, false);

			for (auto it = input.indexedParams.begin() + 1; it != input.indexedParams.end(); ++it)
			{
				out.write(", ", 2);
				table.classdef(*it).print(out, table, false);
			}
		}

		if (not input.namedParams.empty())
		{
			if (not input.indexedParams.empty())
				out.write(", ", 2);

			(out << input.namedParams.front().first).write(": ", 2);
			table.classdef(input.namedParams.front().second).print(out, table, false);

			for (auto it = input.namedParams.begin() + 1; it != input.namedParams.end(); ++it)
			{
				out.write(", ", 2);
				(out << it->first).write(": ", 2);
				table.classdef(it->second).print(out, table, false);
			}
		}
	}


	inline Match FuncOverloadMatch::pushParameter(Atom& atom, yuint32 index, const CLID& clid)
	{
		// force reset
		auto& cdef = table.classdef(clid);
		result.params[index].clid = clid;
		result.params[index].cdef = &cdef;

		// type checking
		auto& paramdef = table.classdef(atom.parameters.vardef(index).clid);

		auto similarity = table.isSimilarTo(nullptr, cdef, paramdef, pAllowImplicit);
		if (similarity == Match::none and canGenerateReport)
		{
			//report.get().trace() << "failed to push value " << cdef.clid << " to parameter "
			//	<< (CLID{atom.atomid,0})
			//	<< " '" << atom.name << "' index " << index;

			if (canGenerateReport)
			{
				String atomName;
				atom.appendCaption(atomName, table);
				auto err = report.get().hint();
				switch (index)
				{
					case 0: err << "1st"; break;
					case 1: err << "2nd"; break;
					case 2: err << "3rd"; break;
					default: err << (index + 1) << "th";
				}
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
		}
		return similarity;
	}


	Match FuncOverloadMatch::validate(Atom& atom, bool allowImplicit)
	{
		// some reset
		result.params.clear();
		result.funcToCall = &atom;

		if (not atom.isFunction())
			return Match::none;

		// trivial check, too many parameters for this overload
		if (atom.parameters.size() < (uint32_t) input.indexedParams.size())
		{
			if (canGenerateReport)
			{
				// do not take into consideration the 'self' parameter for error reporting
				uint selfidx = (atom.isClassMember() and atom.isFunction());
				report.get().hint() << "too many parameters. Got " << (input.indexedParams.size() - selfidx)
					<< ", expected: " << (atom.parameters.size() - selfidx);
			}
			return Match::none;
		}

		pAllowImplicit = allowImplicit;
		bool perfectMatch /*= false*/;

		// checking input parameters
		// determine whether there is at least one parameter, in the atom or pushed
		if (hasAtLeastOneParameter(atom))
		{
			perfectMatch = true;
			result.params.resize(atom.parameters.size());

			// trying to resolve indexed parameters (if they match)
			for (uint i = 0; i != (uint) input.indexedParams.size(); ++i)
			{
				switch (pushParameter(atom, i, input.indexedParams[i]))
				{
					case Match::equal:       perfectMatch = false; break;
					case Match::strictEqual: break;
					case Match::none:        return Match::none;
				}
			}

			// try to resolve named-parameters
			if (not input.namedParams.empty())
			{
				yuint32 offset = (yuint32) input.indexedParams.size();
				for (auto& pair: input.namedParams)
				{
					yuint32 index = atom.parameters.findByName(pair.first, offset);
					// the named parameter is not present after indexed parameters
					if (not (index < atom.parameters.size()))
					{
						//report.trace() << "named parameter '" << pair.first
						//	<< "' not found after started from index " << offset << " in " << (CLID{atom.atomid,0});
						if (canGenerateReport)
							report.get().hint() << "named parameter '" << pair.first << "' not found after indexed parameters";
						return Match::none;
					}

					switch (pushParameter(atom, index, pair.second))
					{
						case Match::equal:       perfectMatch = false; break;
						case Match::strictEqual: break;
						case Match::none:        return Match::none;
					}
				}
			}

			// check for missing default values
			for (size_t i = input.indexedParams.size(); i < result.params.size(); ++i)
			{
				if (nullptr == result.params[i].cdef or result.params[i].cdef->clid.isVoid()) // undefined - TODO: default values
				{
					// std::cout << "the parameter " << i << " is missing - default value not implemented\n";
					if (canGenerateReport)
						report.get().hint() << "the parameter " << i << " is missing";
					return Match::none;
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
					if (Match::none == table.isSimilarTo(nullptr, wantedRettype, atomRettype, pAllowImplicit))
					{
						if (canGenerateReport)
						{
							auto err = report.get().hint() << "returned type does not match, got '";
							atomRettype.print(err.message.message, table, false);
							err << "', expected '";
							wantedRettype.print(err.message.message, table, false);
							err << '\'';
						}
						return Match::none;
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
						if (canGenerateReport)
						{
							auto err = report.get().hint() << "returned type does not match, got '";
							atomRettype.print(err.message.message, table, false);
							err << "', expected 'void'";
						}
						return Match::none;
					}
				}
				break;
			}
			default:
			{
				assert(false and "not implemented");
				return Match::none;
			}
		}
		return perfectMatch ? Match::strictEqual : Match::equal;
	}




} // namespace Nany
