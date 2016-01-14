#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/noncopyable.h>
#include "details/fwd.h"
#include <memory>

// forward declaration
namespace Yuni { namespace Job { class Taskgroup; }}




namespace Nany
{

	class CTarget;
	class BuildInfoSource;
	class BuildInfoContext;



	class Source final
		: public Yuni::IIntrusiveSmartPtr<Source, false, Yuni::Policy::ObjectLevelLockable>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<Source, false, Yuni::Policy::ObjectLevelLockable>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<Source>::Ptr  Ptr;
		//! Threading policy
		typedef Ancestor::ThreadingPolicy ThreadingPolicy;

		enum class Type
		{
			memory,
			file,
		};


	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor, filename or raw content according the input type
		Source(CTarget* target, Type type, AnyString filename, AnyString content);
		//! Copy constructor
		Source(CTarget* target, const Source&);
		//! Destructor
		~Source();
		//@}

		/*!
		** \brief Get if the source is outdated
		*/
		bool isOutdated(yint64& lastModified) const;

		void clean();

		/*!
		** \brief Add a task for building this source
		*/
		void build(BuildInfoContext&, Yuni::Job::Taskgroup& task, Logs::Report& report);

		/*!
		** \brief Build this source
		*/
		bool build(BuildInfoContext&, Logs::Report& report);

		Source& operator = (const Source&) = delete;


	private:
		void resetTarget(CTarget* target);
		//! determine whether the source is outdated or not
		bool isOutdatedWL(yint64& lastModified) const;

		//! build - parse and launch an AST normalization
		// \see duplicateAndNormalizeASTWL()
		bool passASTFromSourceWL();
		//! build - clone and normalize the AST
		bool passDuplicateAndNormalizeASTWL(Logs::Report& report);
		//! build - AST to IR
		bool passTransformASTToIRWL(Logs::Report& report);


	private:
		//! Type of the source
		const Type pType = Type::memory;
		//! Filename (if any)
		Yuni::String pFilename;
		//! Content (content of the file or raw content)
		Yuni::String pContent;
		//! Parent target
		CTarget* pTarget = nullptr;

		//! Date of the last modified
		yint64 pLastCompiled = 0;
		//! Build-related info
		std::unique_ptr<BuildInfoSource> pBuildInfo;


	private:
		friend class CTarget;

	}; // class Source





} // namespace Nany

#include "source.hxx"
