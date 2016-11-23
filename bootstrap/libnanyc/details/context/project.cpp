#include "project.h"
#include "details/vm/runtime/std.core.h"
#include "libnanyc-config.h"

using namespace Yuni;


namespace ny {


CTarget::Ptr Project::doCreateTarget(const AnyString& name) {
	CTarget::Ptr target = new CTarget(self(), name);
	if (!!target) {
		// !!internal: using the target name as reference
		const AnyString& name = target->name();
		// add the target in the project
		targets.all.insert(std::make_pair(name, target));
		// event
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


void Project::init(bool unittests) {
	targets.anonym = doCreateTarget("{default}");
	targets.nsl    = doCreateTarget("{nsl}");
	if (Config::importNSL) {
		importNSLCore(*this);
		if (unittests)
			importNSLUnittests(*this);
	}
	if (cf.on_create)
		cf.on_create(self());
}


void Project::destroy() {
	if (cf.on_destroy)
		cf.on_destroy(self());
	this->~Project();
	auto& allocator = cf.allocator;
	allocator.deallocate(&allocator, this, sizeof(ny::Project));
}


} // namespace ny
