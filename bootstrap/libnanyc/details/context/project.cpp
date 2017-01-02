#include "project.h"
#include "details/vm/runtime/std.core.h"
#include "libnanyc-config.h"

using namespace Yuni;


namespace ny {


Project::Project(const nyproject_cf_t& cf, bool unittests)
	: cf(cf) {
	targets.anonym = doCreateTarget("{default}");
	targets.nsl    = doCreateTarget("{nsl}");
	if (config::importNSL) {
		nsl::import::core(*this);
		if (unittests)
			nsl::import::unittests(*this);
	}
}


Project::~Project() {
	// this container must be destroyed first
	targets.all.clear();
	targets.anonym = nullptr;
	targets.nsl = nullptr;
}


Ref<CTarget> Project::doCreateTarget(const AnyString& name) {
	auto target = make_ref<CTarget>(self(), name);
	if (!!target) {
		// !!internal: using the target name as reference
		const AnyString& name = target->name();
		targets.all.insert(std::make_pair(name, target));
		if (cf.on_target_added)
			cf.on_target_added(self(), target->self(), name.c_str(), name.size());
	}
	return target;
}


void Project::unregisterTargetFromProject(CTarget& target) {
	// remove this target from the list of all targ
	const AnyString& name = target.name();
	// event
	if (cf.on_target_removed)
		cf.on_target_removed(self(), target.self(), name.c_str(), name.size());
	// remove the target from the list of all targets
	targets.all.erase(name);
}


} // namespace ny
