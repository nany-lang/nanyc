#pragma once
#include <yuni/yuni.h>
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/string.h>
#include <nany/nany.h>
#include "source.h"
#include <unordered_map>
#include <unordered_set>

// forward declaration
namespace Yuni { namespace Job { class Taskgroup; }}




namespace Nany
{

	class Project;
	class BuildInfoContext;



	class CTarget final
		: public Yuni::IIntrusiveSmartPtr<CTarget, false, Yuni::Policy::SingleThreaded>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<CTarget, false, Yuni::Policy::SingleThreaded>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<CTarget>::Ptr  Ptr;
		//! Threading policy
		typedef Ancestor::ThreadingPolicy ThreadingPolicy;


	public:
		//! Get if a string is a valid target name
		static bool IsNameValid(const AnyString& name) noexcept;


	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		explicit CTarget(nyproject_t*, const AnyString& name);
		//! Copy constructor
		CTarget(nyproject_t*, const CTarget&);

		//! Destructor
		~CTarget();
		//@}

		//! Target name
		AnyString name() const;

		//! Rename the target
		bool rename(AnyString newname);

		void addSource(const AnyString& name, const AnyString& content);
		void addSourceFromFile(const AnyString& filename);


		void build(BuildInfoContext&, Yuni::Job::Taskgroup& task, Logs::Report& report);

		template<class T> void eachSource(const T& callback);


		//! \name Operators
		//@{
		//! Assignment
		CTarget& operator = (const CTarget&) = delete;
		//@}


	private:
		//! Attached project
		nyproject_t* project = nullptr;

		//! Name of the target
		Yuni::ShortString64 pName;
		//! All sources attached to the target
		std::vector<Source::Ptr> pSources;
		//! All sources ordered by their filename
		std::unordered_map<AnyString, std::reference_wrapper<Source>> pSourcesByFilename;
		//! All sources ordered by their name
		std::unordered_map<AnyString, std::reference_wrapper<Source>> pSourcesByName;

		// friends
		friend class Context;

	}; // class Target





} // namespace nany

#include "target.hxx"
