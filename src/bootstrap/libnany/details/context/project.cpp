#include "project.h"
#include "details/nsl/import-stdcore.h"
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{

	void Project::init()
	{
		targets.anonym = new CTarget(self(), "{default}");
		targets.nsl    = new CTarget(self(), "{nsl}");

		targets.all.insert(std::make_pair(targets.anonym->name(), targets.anonym));
		targets.all.insert(std::make_pair(targets.nsl->name(), targets.nsl));

		if (Config::importNSL)
			importNSLCore(*this);

		if (cf.on_create)
			cf.on_create(self());
	}


	void Project::destroy()
	{
		if (cf.on_destroy)
			cf.on_destroy(self());

		this->~Project();

		auto& allocator = cf.allocator;
		allocator.deallocate(&allocator, this, sizeof(Nany::Project));
	}



} // namespace Nany
