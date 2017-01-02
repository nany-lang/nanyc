#include "details/context/target.h"
#include <yuni/job/queue/service.h>
#include "details/context/project.h"
#include "details/context/build-info.h"
#include <memory>
using namespace Yuni;


namespace ny {


CTarget::CTarget(nyproject_t* project, const AnyString& name)
	: m_project(project)
	, m_name{name} {
}


CTarget::CTarget(nyproject_t* project, const CTarget& rhs)
	: m_project(project)
	, m_name(rhs.m_name) {
	if (not rhs.m_sources.empty()) {
		m_sources.reserve(rhs.m_sources.size());
		for (auto& ptr : rhs.m_sources) {
			auto& source = *ptr;
			auto newSource = make_ref<Source>(this, source);
			m_sources.push_back(newSource);
			auto& ref = *newSource;
			switch (ref.m_type) {
				case Source::Type::memory: {
					m_sourcesByName.insert(std::make_pair(ref.m_filename, std::ref(ref)));
					break;
				}
				case Source::Type::file: {
					m_sourcesByFilename.insert(std::make_pair(ref.m_filename, std::ref(ref)));
					break;
				}
			}
		}
	}
}


CTarget::~CTarget() {
	if (m_project) {
		ny::ref(m_project).unregisterTargetFromProject(*this);
		m_project = nullptr;
	}
	for (auto& ptr : m_sources)
		ptr->resetTarget(nullptr); // detach all sources from parent
	// force cleanup
	m_sourcesByName.clear();
	m_sourcesByFilename.clear();
	m_sources.clear();
}


void CTarget::addSource(const AnyString& name, const AnyString& content) {
	if (not content.empty()) {
		auto source = make_ref<Source>(this, Source::Type::memory, name, content);
		auto it = m_sourcesByName.find(source->m_filename);
		if (it == m_sourcesByName.end())
			m_sourcesByName.insert(std::make_pair(AnyString{source->m_filename}, std::ref(*source)));
		else
			it->second = std::ref(*source);
		m_sources.push_back(source);
	}
}


void CTarget::addSourceFromFile(const AnyString& filename) {
	if (not filename.empty()) {
		auto source = make_ref<Source>(this, Source::Type::file, filename, AnyString());
		auto it = m_sourcesByFilename.find(source->m_filename);
		if (it == m_sourcesByFilename.end())
			m_sourcesByFilename.insert(std::make_pair(AnyString{source->m_filename}, std::ref(*source)));
		else
			it->second = std::ref(*source);
		m_sources.emplace_back(source);
	}
}


} // namespace nany
