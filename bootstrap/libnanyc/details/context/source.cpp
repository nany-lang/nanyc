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


void Source::resetTarget(CTarget* target) {
	ThreadingPolicy::MutexLocker locker{*this};
	m_target = target;
}


void Source::complainEAccess(Build& build) const {
	auto func = build.cf.on_error_file_eacces;
	if (func)
		func(build.project.self(), build.self(), m_filename.c_str(), m_filename.size());
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
	bool success = true;
	yint64 modified = build.buildtime;
	try {
		if (isOutdated(modified)) {
			if (modified <= 0)
				modified = build.buildtime;
			if (m_filename.first() != '{') {
				#ifndef YUNI_OS_WINDOWS
				AnyString arrow {"\u2192 "};
				#else
				constexpr const char* arrow = nullptr;
				#endif
				auto entry = (info() << "building " << m_filename);
				entry.message.prefix = arrow;
			}
			m_details.reset(nullptr); // release memory first
			if (m_type == Type::file) {
				success = (IO::errNone == IO::File::LoadFromFile(m_content, m_filename));
				if (unlikely(not success))
					complainEAccess(build);
			}
			auto report = Logs::Report{*build.messages} .subgroup();
			report.data().origins.location.filename = m_filename;
			report.data().origins.location.target.clear();
			if (success) {
				m_details = std::make_unique<BuildInfoSource>(build.cf);
				success &= passASTFromSourceWL();
				success &= passDuplicateAndNormalizeASTWL(report);
				success &= passTransformASTToIRWL(report);
				success = success and build.attach(m_details->parsing.sequence);
				m_details->parsing.success = success;
			}
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
	build.success = success;
	m_lastCompiled = success ? modified : 0;
	return success;
}


} // namespace ny
