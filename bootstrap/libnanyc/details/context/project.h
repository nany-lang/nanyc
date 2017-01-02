#pragma once
#include "nany/nany.h"
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/thread/mutex.h>
#include <map>
#include "target.h"



namespace ny {

class Project final
	: public Yuni::IIntrusiveSmartPtr<Project, false, Yuni::Policy::SingleThreaded> {
public:
	explicit Project(const nyproject_cf_t& cf, bool unittests);
	Project(const Project&) = delete;
	~Project();

	nyproject_t* self();
	const nyproject_t* self() const;

	//! Allocate a new object
	template<class T, typename... Args> T* allocate(Args&& ... args);
	//! delete an object
	template<class T> void deallocate(T* object);


	Project& operator = (const Project&) = delete;


public:
	//! User settings
	nyproject_cf_t cf;

	struct {
		//! Default target
		yuni::Ref<CTarget> anonym;
		//! Default target
		yuni::Ref<CTarget> nsl;
		//! All other targets
		std::map<AnyString, yuni::Ref<CTarget>> all;
	}
	targets;

	//! Mutex
	mutable Yuni::Mutex mutex;


private:

	yuni::Ref<CTarget> doCreateTarget(const AnyString& name);
	void unregisterTargetFromProject(CTarget& target);
	friend class CTarget;

}; // class Project




//! Convert a nyproject_t into a ny::Project
Project& ref(nyproject_t* const);
//! Convert a nyproject_t into a ny::Project
const Project& ref(const nyproject_t* const);


} // namespace ny

#include "project.hxx"
