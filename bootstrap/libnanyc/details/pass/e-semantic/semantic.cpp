#include "details/context/build.h"
#include "details/semantic/semantic-analysis.h"
#include "details/semantic/atom-factory.h"
#include "libnanyc-traces.h"

using namespace yuni;


namespace ny {


bool Build::resolveStrictParameterTypes(Atom& atom) {
	return ny::semantic::resolveStrictParameterTypes(compdb, atom);
}


bool Build::instanciate(const AnyString& entrypoint, const CType* args, uint32_t& atomid,
		uint32_t& instanceid) {
	ny::Logs::Report report{compdb.messages};
	if (unlikely(args)) {
		report.error() << "arguments for atom instanciation is not supported yet";
		return false;
	}
	MutexLocker locker{mutex};
	Atom* entrypointAtom = nullptr;
	try {
		compdb.cdeftable.atoms.root.eachChild(entrypoint, [&](Atom & child) -> bool {
			if (unlikely(entrypointAtom != nullptr))
				throw std::runtime_error("': multiple entry points found");
			entrypointAtom = &child;
			return true;
		});
		if (unlikely(!entrypointAtom))
			throw std::runtime_error("()': function not found");
		if (unlikely(not entrypointAtom->isFunction() or entrypointAtom->isClassMember()))
			throw std::runtime_error("': the atom is not a function");
	}
	catch (const std::exception& e) {
		report.error() << "failed to instanciate '" << entrypoint << e.what();
		return false;
	}
	decltype(ny::semantic::FuncOverloadMatch::result.params) params;
	decltype(ny::semantic::FuncOverloadMatch::result.params) tmplparams;
	ClassdefTableView cdeftblView{compdb.cdeftable};
	ny::semantic::Settings settings {
		*entrypointAtom, cdeftblView, compdb, params, tmplparams
	};
	bool instanciated = ny::semantic::instanciateAtom(settings);
	report.appendEntry(settings.report);
	if (config::traces::atomTable)
		compdb.cdeftable.atoms.root.printTree(compdb.cdeftable);
	if (not instanciated) {
		atomid = (uint32_t) - 1;
		instanceid = (uint32_t) - 1;
		return false;
	}
	atomid = entrypointAtom->atomid;
	instanceid = settings.instanceid;
	return true;
}


} // namespace ny
