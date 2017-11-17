#include "details/atom/classdef-table-view.h"
#include "stack-frame.h"
#include "details/errors/errors.h"

namespace ny::semantic {

DelayedReportOnRaise::DelayedReportOnRaise(Atom& atom)
	: m_atom(atom) {
}

void DelayedReportOnRaise::addRaisedErrors(yuni::String&& type, DelayedReportOnRaise::RaiseOrigins&& origins) {
	m_noHandlerPerTypename[type].emplace_back(std::move(origins));
}

void DelayedReportOnRaise::produceErrors() const {
	for (auto& pair: m_noHandlerPerTypename) {
		auto err = (error() << "no error handler provided for '" << pair.first << '\'');
		for (auto& report: pair.second) {
			auto fromCall = err.hint();
			fromCall << "required for '" << report.atomname << '\'';
			fromCall.origins().location.filename = m_atom.origin.filename;
			fromCall.origins().location.pos.line = report.line;
			fromCall.origins().location.pos.offset = report.offset;
			for (auto& origin: report.origins) {
				auto h = (fromCall.hint() << "raised from '");
				h << origin.atomname << '\'';
				h.origins().location.filename = origin.filename;
				h.origins().location.pos.line = origin.line;
				h.origins().location.pos.offset = origin.offset;
			}
		}
	}
}

AtomStackFrame::~AtomStackFrame() {
	if (unlikely(m_delayedErrorsOnRaise))
		m_delayedErrorsOnRaise->produceErrors();
}

void AtomStackFrame::addRaisedErrors(yuni::String&& type, DelayedReportOnRaise::RaiseOrigins&& origins) {
	if (!m_delayedErrorsOnRaise)
		m_delayedErrorsOnRaise = std::make_unique<DelayedReportOnRaise>(atom);
	m_delayedErrorsOnRaise->addRaisedErrors(std::move(type), std::move(origins));
}

} // ny::semantic
