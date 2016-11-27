#include <yuni/yuni.h>
#include <yuni/thread/utility.h>
#include <yuni/job/taskgroup.h>
#include <yuni/io/file.h>
#include "source.h"
#include "build-info.h"
#include "target.h"
#include "details/errors/errors.h"
#include <memory>
#include "details/context/build.h"

using namespace Yuni;


namespace ny {


Source::Source(CTarget* target, Source::Type type, AnyString filename, AnyString content)
	: m_type{type}
	, m_content{content}
	, m_target(target) {
	switch (type) {
		case Type::file:
			IO::Canonicalize(m_filename, filename);
			break;
		case Type::memory:
			m_filename = filename;
			break;
	}
}


Source::Source(CTarget* target, const Source& rhs) // assuming that rhs is already protected
	: m_type(rhs.m_type)
	, m_filename(rhs.m_filename)
	, m_content(rhs.m_content)
	, m_target(target) {
}


Source::~Source() {
	// keep the symbol local
}


bool Source::isOutdated(yint64& lastModified) const {
	if (m_type == Type::file) {
		auto lmt = IO::File::LastModificationTime(m_filename);
		if (lmt != m_lastCompiled) {
			lastModified = lmt;
			return true;
		}
		return false;
	}
	lastModified = m_lastCompiled;
	return (m_lastCompiled <= 0);
}


bool Source::build(Build& build) {
	bool success = false;
	yint64 modified = build.buildtime;
	try {
		if (not isOutdated(modified)) {
			success = true; // not modified
		}
		else {
			if (modified <= 0)
				modified = build.buildtime;
			// reporting
			if (m_filename.first() != '{') {
				#ifndef YUNI_OS_WINDOWS
				AnyString arrow {"\u2192 "};
				#else
				constexpr const char* arrow = nullptr;
				#endif
				auto entry = (info() << "building " << m_filename);
				entry.message.prefix = arrow;
			}
			// assuming it will succeed, will be reverted to false as soon as something goes wrong
			success = true;
			if (m_type == Type::file) {
				m_content.clear();
				m_content.shrink();
				success = (IO::errNone == IO::File::LoadFromFile(m_content, m_filename));
				if (unlikely(not success and m_target)) {
					auto f = build.cf.on_error_file_eacces;
					if (f)
						f(build.project.self(), build.self(), m_filename.c_str(), m_filename.size());
				}
			}
			// create a new report entry
			auto report = Logs::Report{*build.messages} .subgroup();
			// reset build-info
			report.data().origins.location.filename = m_filename;
			report.data().origins.location.target.clear();
			pBuildInfo.reset(nullptr); // making sure that the memory is released first
			pBuildInfo = std::make_unique<BuildInfoSource>(build.cf);
			if (success) { // file not opened
				// creates an AST from source code
				success &= passASTFromSourceWL();
				// duplicates the AST and normalize it on-the-fly
				success &= passDuplicateAndNormalizeASTWL(report);
				// uses the normalized AST to generate high-level nany-IR
				success &= passTransformASTToIRWL(report);
				// attach the new sequence to the execution context
				if (success) {
					auto& sequence = pBuildInfo->parsing.sequence;
					success &= build.attach(sequence);
				}
			}
			// keep the result of the process somewhere
			pBuildInfo->parsing.success = success;
		}
	}
	catch (std::bad_alloc&) {
		build.printStderr("ice: not enough memory");
		success = false;
	}
	catch (...) {
		ice() << "uncaught exception when building '" << m_filename << "'";
		success = false;
	}
	if (not success) {
		m_lastCompiled = 0; // error
		// update the global status
		build.success = false;
	}
	else
		m_lastCompiled = modified;
	return success;
}


} // namespace ny
