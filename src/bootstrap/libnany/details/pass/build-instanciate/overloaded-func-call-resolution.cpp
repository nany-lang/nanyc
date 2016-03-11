#include "overloaded-func-call-resolution.h"
#include "instanciate.h"
#include "instanciate-atom.h"
#include "libnany-config.h"




namespace Nany
{

	bool OverloadedFuncCallResolver::resolve(const std::vector<std::reference_wrapper<Atom>>& solutions)
	{
		// DEFINITION: suitable solution:
		// a solution which offers a prototype that matches the input parameters of the func call
		//
		// DEFINITION: perfect match:
		//   a perfect match is a prototype that strictly satisfies the input parameter types
		//   (aka the appropriate type has been given, not "any" nor something that "looks like" the same)

		// invalidating the result
		atom = nullptr;
		params = nullptr;
		// alias
		uint32_t solutionCount = static_cast<uint32_t>(solutions.size());

		// all really suitable solutions (type match)
		suitable.clear();
		suitable.resize(solutionCount, false);
		// sub-reports for error generation
		subreports.clear();
		subreports.resize(solutionCount);
		// parameter instanciation per solution
		parameters.clear();
		parameters.resize(solutionCount);

		if (unlikely(solutionCount == 0))
			return false;

		// the last perfect match found
		Atom* perfectMatch = nullptr;
		ParameterTypesRequested* perfectMatchParams = nullptr;
		// flag for determine whether a perfect match has really been found (and not invalidated a posteriori)
		uint32_t perfectMatchCount = 0;

		overloadMatch.canGenerateReport = false;

		// trying to find all suitable solutions
		for (uint32_t r = 0; r != solutionCount; ++r)
		{
			Atom& atomsol = solutions[r].get();

			auto match = overloadMatch.validate(atomsol);
			switch (match)
			{
				case TypeCheck::Match::none:
				{
					break;
				}
				case TypeCheck::Match::equal:
				case TypeCheck::Match::strictEqual:
				{
					// ok, the solution is "suitable"
					++suitableCount;
					suitable[r] = true;
					atom = &atomsol;
					// keeping the new parameters
					// for using this solution, implicit object creation may be required
					parameters[r].first.swap(overloadMatch.result.params);
					parameters[r].second.swap(overloadMatch.result.tmplparams);
					params     = &(parameters[r].first);
					tmplparams = &(parameters[r].second);

					// Found a perfect match ! Keeping traces of it to reuse it later if it is _the_ solution
					// (and if unique)
					if (match == TypeCheck::Match::strictEqual)
					{
						perfectMatch = atom;
						perfectMatchParams = params;
						++perfectMatchCount;
					}
					break;
				}
			}
		}

		if (unlikely(0 == suitableCount)) // sorry. nothing.
		{
			if (false)
			{
				// let's do another loop for generating a report (currently done by the caller)
				overloadMatch.canGenerateReport = true;
				auto err = (report.error() << "cannot call ");
				overloadMatch.report = std::ref(err);

				for (uint32_t r = 0; r != solutionCount; ++r)
					overloadMatch.validate(solutions[r].get());
			}
			return false;
		}

		if (suitableCount > 1)
		{
			if (1 == perfectMatchCount)
			{
				// several solutions might be possible, but a perfect match has been found
				atom = perfectMatch; // this is _the_ solution !
				params = perfectMatchParams;
				suitableCount = 1;
			}
			else
			{
				// since we have several suitable solutions (types are not enough stricts),
				// all func calls will be instanciated

				// the total number of instanciation that succeeded
				uint instanceSuccessCount = 0;
				// keeping traces of solutions that can be instanciated
				solutionsThatCanBeInstanciated.clear();
				solutionsThatCanBeInstanciated.resize(suitable.size(), false);

				for (uint32_t r = 0; r != (uint32_t) suitable.size(); ++r)
				{
					if (not suitable[r])
						continue;

					// reporting for the instanciation (may or may not be created by `instanciateAtom`)
					Logs::Message::Ptr newReport;
					auto& solutionAtom = solutions[r].get();

					// trying to instanciate the solution
					Pass::Instanciate::InstanciateData info{
						newReport, solutionAtom, cdeftable, context,
						parameters[r].first, parameters[r].second
					};
					info.canGenerateCode = canGenerateCode;
					info.canGenerateErrors = canGenerateErrors;

					if (InstanciateAtom(info))
					{
						// nice, it works ! Keeping it (as the last good solution)
						++instanceSuccessCount;
						atom = &(solutions[r].get());
						params = &(parameters[r].first);
						tmplparams = &(parameters[r].second);
						solutionsThatCanBeInstanciated[r] = true;

						// keep the report if not empty
						if (!(!newReport) and not (newReport->entries.empty() and newReport->level == Logs::Level::none))
							subreports[r].swap(newReport);
					}
					else
					{
						// keeping the error report for the user
						subreports[r].swap(newReport);
					}
				}

				if (instanceSuccessCount > 0 and instanceSuccessCount < suitableCount)
				{
					// actually only a part of the initial solutions can be instanciated, so
					// there is no real error, just an ambigious call
					for (uint r = 0; r != (uint)suitable.size(); ++r)
					{
						// keeping only results that can be instanciated
						suitable[r] = suitable[r] and solutionsThatCanBeInstanciated[r];
					}
				}

				// the total number of suitable strictly follows the number of func calls
				// that could have been instanciatied
				suitableCount = instanceSuccessCount;
			}
		}

		return (1 == suitableCount);
	}





} // namespace Nany
