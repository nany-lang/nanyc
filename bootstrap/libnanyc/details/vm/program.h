#pragma once
#include <yuni/yuni.h>
#include <yuni/core/smartptr/intrusive.h>
#include "details/ir/sequence.h"
#include "nany/nany.h"
#include "details/atom/atom-map.h"
#include "details/context/build.h"
#include "libnanyc-config.h"



namespace ny {
namespace vm {

class Program final
	: public Yuni::IIntrusiveSmartPtr<Build, false, Yuni::Policy::SingleThreaded> {
public:
	//! Intrusive Smarptr
	using IntrusivePtr = IIntrusiveSmartPtr<Build, false, Yuni::Policy::SingleThreaded>;
	//! The threading policy of this object
	using ThreadingPolicy = IntrusivePtr::ThreadingPolicy;


public:
	/*!
	** \brief Create a brand new program
	*/
	Program(const nyprogram_cf_t&, nybuild_t*);

	void destroy();

	/*!
	** \brief Execute the main entry point
	*/
	int execute(uint32_t argc, const char** argv);

	/*!
	** \brief Print a message on the console
	*/
	void printStderr(const AnyString& msg);

	nyprogram_t* self();
	const nyprogram_t* self() const;


public:
	//! Settings
	nyprogram_cf_t cf;
	//! User build
	nybuild_t* build;
	//! Atom Map
	const AtomMap& map;

	//! Return value, a pod or an object
	int retvalue = 0;
	//! Does the program own the sequence ?
	bool ownsSequence = false;


private:
	//! Destructor, should use destroy instead
	~Program();

}; // class Program


} // namespace vm


//! Convert a nyproject_t into a ny::Project
vm::Program& ref(nyprogram_t* const);
//! Convert a nyproject_t into a ny::Project
const vm::Program& ref(const nyprogram_t* const);


} // namespace ny

#include "program.hxx"
