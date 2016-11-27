#pragma once
#include "libnanyc.h"
#include <yuni/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/noncopyable.h>
#include <memory>


namespace Yuni { namespace Job { class Taskgroup; } }


namespace ny {

class CTarget;
class BuildInfoSource;
class Build;
namespace Logs { class Report; }



class Source final
	: public Yuni::IIntrusiveSmartPtr<Source, false, Yuni::Policy::SingleThreaded> {
public:
	//! The class ancestor
	using Ancestor = Yuni::IIntrusiveSmartPtr<Source, false, Yuni::Policy::SingleThreaded>;
	//! The most suitable smart ptr for the class
	using Ptr = Ancestor::SmartPtrType<Source>::Ptr;
	//! Threading policy
	using ThreadingPolicy = Ancestor::ThreadingPolicy;

	enum class Type {
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

	//! Get if the source is outdated
	bool isOutdated(yint64& lastModified) const;
	//! Build this source
	bool build(Build&);

	Source& operator = (const Source&) = delete;


private:
	void resetTarget(CTarget* target);
	//! build - parse and launch an AST normalization
	// \see duplicateAndNormalizeASTWL()
	bool passASTFromSourceWL();
	//! build - clone and normalize the AST
	bool passDuplicateAndNormalizeASTWL(Logs::Report& report);
	//! build - AST to IR
	bool passTransformASTToIRWL(Logs::Report& report);

private:
	//! Type of the source
	const Type m_type = Type::memory;
	//! Filename (if any)
	Yuni::String m_filename;
	//! Content (content of the file or raw content)
	Yuni::String m_content;
	//! Parent target
	CTarget* m_target = nullptr;
	//! Date of the last modified
	yint64 m_lastCompiled = 0;
	//! Build-related info
	std::unique_ptr<BuildInfoSource> m_details;

private:
	friend class CTarget;

}; // class Source


} // namespace ny

#include "source.hxx"
