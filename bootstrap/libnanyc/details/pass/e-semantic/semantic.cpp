#include "details/context/build.h"
#include "details/semantic/semantic-analysis.h"
#include "details/semantic/atom-factory.h"
#include "libnanyc-traces.h"

using namespace yuni;


namespace ny {


bool Build::resolveStrictParameterTypes(Atom& atom) {
	return ny::semantic::resolveStrictParameterTypes(*this, atom);
}


bool Build::instanciate(const AnyString& entrypoint, const nytype_t* args, uint32_t& atomid,
		uint32_t& instanceid) {
	ny::Logs::Report report{*messages.get()};
	if (unlikely(args)) {
		report.error() << "arguments for atom instanciation is not supported yet";
		return false;
	}
	MutexLocker locker{mutex};
	Atom* entrypointAtom = nullptr;
	try {
		cdeftable.atoms.root.eachChild(entrypoint, [&](Atom & child) -> bool {
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
	std::shared_ptr<Logs::Message> newReport;
	ClassdefTableView cdeftblView{cdeftable};
	ny::semantic::InstanciateData info {
		newReport, *entrypointAtom, cdeftblView, *this, params, tmplparams
	};
	bool instanciated = ny::semantic::instanciateAtom(info);
	report.appendEntry(newReport);
	if (config::traces::atomTable)
		cdeftable.atoms.root.printTree(cdeftable);
	if (not instanciated) {
		atomid = (uint32_t) - 1;
		instanceid = (uint32_t) - 1;
		return false;
	}
	atomid = entrypointAtom->atomid;
	instanceid = info.instanceid;
	return true;
}


} // namespace ny
