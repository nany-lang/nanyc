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
	using IntrusiveSmartPtr = Yuni::IIntrusiveSmartPtr<Project, false, Yuni::Policy::SingleThreaded>;
	using Ptr = IntrusiveSmartPtr::Ptr;

public:
	//! Ctor with an user-defined settings
	explicit Project(const nyproject_cf_t& cf);
	//! Copy ctor
	Project(const Project&) = delete;

	//! Initialize the project (after the ref count has been incremented)
	void init(bool unittests);
	//! Call the destructor and release this
	void destroy();


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
		CTarget::Ptr anonym;
		//! Default target
		CTarget::Ptr nsl;

		//! All other targets
		std::map<AnyString, CTarget::Ptr> all;
	}
	targets;

	//! Mutex
	mutable Yuni::Mutex mutex;


private:
	//! Destructor, private, destroy() must be used instead
	~Project();

	CTarget::Ptr doCreateTarget(const AnyString& name);
	void unregisterTargetFromProject(CTarget& target);
	friend class CTarget;

}; // class Project




//! Convert a nyproject_t into a ny::Project
Project& ref(nyproject_t* const);
//! Convert a nyproject_t into a ny::Project
const Project& ref(const nyproject_t* const);


} // namespace ny

#include "project.hxx"
