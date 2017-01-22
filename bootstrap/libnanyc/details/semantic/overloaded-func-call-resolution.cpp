#include "overloaded-func-call-resolution.h"
#include "semantic-analysis.h"
#include "atom-factory.h"


namespace ny {
namespace semantic {


bool OverloadedFuncCallResolver::resolve(const std::vector<std::reference_wrapper<Atom>>& solutions) {
	// DEFINITION: suitable solution:
	// a solution which offers a prototype that matches the input parameters of the func call
	//
	// DEFINITION: perfect match:
	//   a perfect match is a prototype that strictly satisfies the input parameter types
	//   (aka the appropriate type has been given, not "any" nor something that "looks like" the same)
	// invalidating the result
	atom = nullptr;
	params = nullptr;
	suitable.clear();
	scores.clear();
	subreports.clear();
	parameters.clear();
	uint32_t solutionCount = static_cast<uint32_t>(solutions.size());
	if (unlikely(solutionCount == 0))
		return false;
	// all really suitable solutions (type match)
	suitable.resize(solutionCount, false);
	// all scores for each solution
	scores.resize(solutionCount, 0u);
	// sub-reports for error generation
	subreports.resize(solutionCount);
	// parameter instanciation per solution
	parameters.resize(solutionCount);
	// the last perfect match found
	Atom* perfectMatch = nullptr;
	ParameterTypesRequested* perfectMatchParams = nullptr;
	ParameterTypesRequested* perfectMatchTmplParams = nullptr;
	// flag for determine whether a perfect match has really been found (and not invalidated a posteriori)
	uint32_t perfectMatchCount = 0;
	// trying to find all suitable solutions
	for (uint32_t r = 0; r != solutionCount; ++r) {
		Atom& atomsol = solutions[r].get();
		auto match = overloadMatch.validate(atomsol);
		switch (match) {
			case TypeCheck::Match::none: {
				break;
			}
			case TypeCheck::Match::equal:
			case TypeCheck::Match::strictEqual: {
				// ok, the solution is "suitable"
				++suitableCount;
				suitable[r] = true;
				scores[r] = overloadMatch.result.score;
				atom = &atomsol;
				// keeping the new parameters
				// for using this solution, implicit object creation may be required
				parameters[r].first.swap(overloadMatch.result.params);
				parameters[r].second.swap(overloadMatch.result.tmplparams);
				params     = &(parameters[r].first);
				tmplparams = &(parameters[r].second);
				// Found a perfect match ! Keeping traces of it to reuse it later if it is _the_ solution
				// (and if unique)
				if (match == TypeCheck::Match::strictEqual) {
					perfectMatch = atom;
					perfectMatchParams = params;
					perfectMatchTmplParams = tmplparams;
					++perfectMatchCount;
				}
				break;
			}
		}
	}
	if (unlikely(0 == suitableCount)) { // sorry. nothing.
		if (false) {
			// let's do another loop for generating a report (currently done by the caller)
			auto err = (report.error() << "cannot call ");
			for (uint32_t r = 0; r != solutionCount; ++r) {
				overloadMatch.report = &err;
				overloadMatch.validateWithErrReport(solutions[r].get());
			}
		}
		return false;
	}
	if (suitableCount > 1) {
		// Several suitable solutions are available. Not all types are strict enough and
		// several overloads are possible matches. In this case, it is still possible to get
		// a perfect match is a single solution has the best score (the score is merely the
		// number of parameters that match perfectly)
		//
		// see description of the OverloadedFuncCallResolver::scores
		if (0 == perfectMatch) {
			uint32_t bestScoreIndex = 0u;
			uint32_t bestScore = 0u;
			bool unique = true;
			assert(scores.size() == suitable.size());
			for (uint32_t r = 0; r != solutionCount; ++r) {
				if (suitable[r]) {
					uint32_t s = scores[r];
					if (s > bestScore) {
						unique = true;
						bestScore = s;
						bestScoreIndex = r;
					}
					else if (s == bestScore)
						unique = false;
				}
			}
			if (0 != bestScore and unique) { // a perfect match has been found
				perfectMatchCount = 1u;
				perfectMatch = &(solutions[bestScoreIndex].get());
				perfectMatchParams     = &(parameters[bestScoreIndex].first);
				perfectMatchTmplParams = &(parameters[bestScoreIndex].second);
			}
		}
		if (1 == perfectMatchCount) {
			// several solutions might be possible, but a perfect match has been found
			atom = perfectMatch; // this is _the_ solution !
			params = perfectMatchParams;
			tmplparams = perfectMatchTmplParams;
			suitableCount = 1;
		}
		else {
			// not able to find asuitable solution from the input types
			// trying to instanciate all func calls to see if one of those
			// is the good one
			// the total number of instanciation that succeeded
			uint instanceSuccessCount = 0;
			// keeping traces of solutions that can be instanciated
			solutionsThatCanBeInstanciated.clear();
			solutionsThatCanBeInstanciated.resize(suitable.size(), false);
			for (uint32_t r = 0; r != solutionCount; ++r) {
				if (not suitable[r])
					continue;
				// reporting for the instanciation (may or may not be created by `instanciateAtom`)
				std::shared_ptr<Logs::Message> newReport;
				auto& solutionAtom = solutions[r].get();
				// trying to instanciate the solution
				ny::semantic::InstanciateData info {
					newReport, solutionAtom, cdeftable, build, parameters[r].first, parameters[r].second
				};
				info.canGenerateCode = canGenerateCode;
				info.canGenerateErrors = canGenerateErrors;
				info.parent = parent;
				if (instanciateAtom(info)) {
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
				else {
					// keeping the error report for the user
					subreports[r].swap(newReport);
				}
			}
			if (instanceSuccessCount > 0 and instanceSuccessCount < suitableCount) {
				// actually only a part of the initial solutions can be instanciated, so
				// there is no real error, just an ambigious call
				for (uint r = 0; r != (uint)suitable.size(); ++r) {
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


} // namespace semantic
} // namespace ny
