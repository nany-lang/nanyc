#pragma once
#include "nany/nany.h"
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/atomic/bool.h>
#include "project.h"
#include "source.h"
#include "details/reporting/message.h"
#include "details/atom/classdef-table.h"
#include "details/intrinsic/intrinsic-table.h"



namespace ny {

class Build final
	: public Yuni::IIntrusiveSmartPtr<Build, false, Yuni::Policy::SingleThreaded> {
public:
	//! Intrusive Smarptr
	using IntrusivePtr = IIntrusiveSmartPtr<Build, false, Yuni::Policy::SingleThreaded>;
	//! The threading policy of this object
	using ThreadingPolicy = IntrusivePtr::ThreadingPolicy;


public:
	struct AttachedSequenceRef final {
		AttachedSequenceRef(ir::Sequence* sequence, bool owned)
			: sequence(sequence)
			, owned(owned) {
		}
		AttachedSequenceRef(AttachedSequenceRef&&) = default;
		AttachedSequenceRef(const AttachedSequenceRef&) = delete;
		~AttachedSequenceRef();

		ir::Sequence* sequence = nullptr;
		bool owned = false;
	};


public:
	explicit Build(Project& project, const nybuild_cf_t& cf, bool async);
	~Build();

	//! Initialize the project (after the ref count has been incremented)
	void init();

	/*!
	** \brief Compile the project
	*/
	bool compile();

	/*!
	** Attach an IR sequence
	*/
	bool attach(ir::Sequence& sequence, bool owned = false);

	/*!
	** \brief Try to resolve strict parameter types
	*/
	bool resolveStrictParameterTypes(Atom&);

	/*!
	** \brief Try to instanciate an entry point
	*/
	bool instanciate(const AnyString& entrypoint, const nytype_t* args, uint32_t& atomid, uint32_t& instanceid);


	/*!
	** \brief Print a message on the console
	*/
	void printStderr(const AnyString& msg);
	/*!
	** \brief Print a message on the console
	*/
	void cerrColor(nycolor_t);


	//! Allocate a new object
	template<class T, typename... Args> T* allocate(Args&& ... args);
	//! Allocate a new memory region
	template<class T> T* allocateraw(size_t size);
	//! delete an object
	template<class T> void deallocate(T* object);
	//! delete a memory region
	void deallocate(void* object, size_t size);

	//! Get the equivalent C pointer
	nybuild_t* self();
	//! Get the equivalent C pointer (const)
	const nybuild_t* self() const;


public:
	//! Lock for concurrent access to the classef table
	Yuni::Mutex mutex;
	//! User settings
	nybuild_cf_t cf;
	//! Attached project
	Project& project;

	//! Canceled flag
	Yuni::Atomic::Bool canceled;

	//! The datatype matrix
	ClassdefTable cdeftable;

	//! All intrinsics
	IntrinsicTable intrinsics;

	//! All report messages
	std::unique_ptr<Logs::Message> messages;

	//! Build time
	int64_t buildtime = 0;
	//! Duration (in ms)
	int64_t duration = 0;

	//! Flag to determine whether the build should be async or not
	const bool isAsync;
	//! success
	bool success = false;

	struct {
		uint32_t atomid = (uint32_t) - 1;
		uint32_t instanceid = (uint32_t) - 1;
	}
	main;

private:
	std::vector<std::reference_wrapper<Source>> m_sources;
	std::vector<yuni::Ref<CTarget>> m_targets;
	std::vector<AttachedSequenceRef> m_attachedSequences;

}; // class Build






//! Convert a nyproject_t into a ny::Project
Build& ref(nybuild_t* const);
//! Convert a nyproject_t into a ny::Project
const Build& ref(const nybuild_t* const);



} // namespace ny

#include "build.hxx"
