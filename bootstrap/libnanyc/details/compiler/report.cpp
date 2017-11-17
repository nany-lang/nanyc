#include "details/compiler/report.h"
#include "details/errors/errors.h"
#include "details/atom/classdef-table-view.h"

namespace ny::compiler::report {

namespace {

std::pair<yuni::String, uint32_t> exportRaisedErrorsAtom(const ClassdefTableView& cdeftable, const Atom& atom, uint32_t depth) {
	yuni::String out;
	for (uint32_t i = 0; i != depth; ++i)
		out += ".   ";
	out << atom.keyword() << ' ' << atom.caption(cdeftable);
	uint32_t count = 0;
	if (not atom.funcinfo.raisedErrors.empty()) {
		if (not atom.funcinfo.raisedErrors.empty()) {
			atom.funcinfo.raisedErrors.each([&](const Atom& type, auto& origins) {
				count += 1;
				out << '\n';
				for (uint32_t i = 0; i != depth; ++i)
					out += ".   ";
				out << '\t' << type.keyword() << ' ' << type.caption(cdeftable);
				for (auto& origin: origins) {
					out << '\n';
					for (uint32_t i = 0; i != depth; ++i)
						out += ".   ";
					out << "\t  \\- ";
					if (origin.atom.atomid == atom.atomid)
						out << "raise ";
					else
						out << "leak from '" << origin.atom.keyword() << ' ' << origin.atom.caption(cdeftable) << "' ";
					out << origin.atom.origin.filename << ':' << origin.line << ':' << origin.column;
				}
			});
		}
	}
	atom.eachChild([&](const Atom& child) -> bool {
		auto r = exportRaisedErrorsAtom(cdeftable, child, depth + 1);
		if (r.second != 0) {
			out << '\n' << r.first;
			count += r.second;
		}
		return true;
	});
	if (count == 0)
		out.clear();
	return std::make_pair(out, count);
}

} // namespace

void raisedErrorsForAllAtoms(ny::compiler::Compdb& compdb, ny::Logs::Report& report) {
	auto tr = (report.trace() << "raised errors summary (");
	uint32_t count = 0;
	ClassdefTableView view(compdb.cdeftable);
	compdb.cdeftable.atoms.root.eachChild([&](const Atom& child) -> bool {
		if (not child.isUnit()) {
			auto r = exportRaisedErrorsAtom(view, child, 0);
			if (r.second != 0) {
				trace() << r.first;
				count += r.second;
			}
		}
		return true;
	});
	tr << count << " found)";
}

} // ny::compiler::report
