#include "overloaded-func-call-resolution.h"
#include "instanciate.h"
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
		//   (aka the appropriate type has been given, not "any" nor something that looks like)

		// invalidating the result
		atom = nullptr;
		params = nullptr;

		// all really suitable solutions (type match)
		suitable.clear();
		suitable.resize(solutions.size(), false);
		// sub-reports for error generation
		subreports.clear();
		subreports.resize(solutions.size());
		// parameter instanciation per solution
		parameters.clear();
		parameters.resize(solutions.size());

		// the last perfect match found
		Atom* perfectMatch = nullptr;
		ParameterTypesRequested* perfectMatchParams = nullptr;
		// flag for determine whether a perfect match has really been found (and not invalidated a posteriori)
		uint perfectMatchCount = 0;

		overloadMatch.canGenerateReport = false;

		// trying to find all suitable solutions
		for (uint r = 0; r != (uint) solutions.size(); ++r)
		{
			Atom& atomsol = solutions[r].get();

			// checking for each parameter
			auto match = overloadMatch.validate(atomsol);
			if (match == TypeCheck::Match::none) // hum no. sorry.
				continue;

			// ok, the solution is "suitable"
			++suitableCount;
			suitable[r] = true;
			atom = &atomsol;
			// keeping the new parameters
			// for using this solution, implicit object creation may be required
			parameters[r].swap(overloadMatch.result.params);
			params = &(parameters[r]);

			// Found a perfect match ! Keeping traces of it to reuse it later if it is _the_ solution
			// (and if unique)
			if (match == TypeCheck::Match::strictEqual)
			{
				perfectMatch = atom;
				perfectMatchParams = params;
				++perfectMatchCount;
			}
		}

		if (0 == suitableCount) // sorry. nothing.
			return false;

		if (suitableCount > 1)
		{
			if (1 == perfectMatchCount)
			{
				// several solutions might be possible, but a single perfect match has been found
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

				for (uint r = 0; r != (uint) suitable.size(); ++r)
				{
					if (not suitable[r])
						continue;

					// reporting for the instanciation (may or may not be created by `instanciateAtom`)
					Logs::Message::Ptr newReport;

					// trying to instanciate the solution
					Pass::Instanciate::InstanciateData info{newReport, solutions[r].get(), cdeftable, intrinsics, parameters[r]};
					info.canGenerateCode = canGenerateCode;
					info.canGenerateErrors = canGenerateErrors;

					if (InstanciateAtom(info))
					{
						// nice, it works ! Keeping it (as the last good solution)
						++instanceSuccessCount;
						atom = &(solutions[r].get());
						params = &(parameters[r]);
						solutionsThatCanBeInstanciated[r] = true;

						// keep the report if not empty
						if (!(!newReport) and not (newReport->entries.empty() and newReport->level == Logs::Level::none))
							subreports[r].swap(newReport);
					}
					else
					{
						// keeping the error report, for reporting it to the user
						subreports[r].swap(newReport);
					}
				}

				if (instanceSuccessCount > 0 and instanceSuccessCount < suitableCount)
				{
					// actually only a part of the initial solutions can be instanciated, so
					// there is no real error, just an ambigious call
					for (uint r = 0; r != (uint)suitable.size(); ++r)
						suitable[r] = suitable[r] and solutionsThatCanBeInstanciated[r]; // keeping only results that can be instanciated
				}

				// the total number of suitable strictly follows the number of func calls
				// that could have been instanciatied
				suitableCount = instanceSuccessCount;
			}
		}

		return (1 == suitableCount);
	}





} // namespace Nany
