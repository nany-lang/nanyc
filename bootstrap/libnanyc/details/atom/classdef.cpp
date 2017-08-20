#include "details/atom/classdef.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/atom/atom.h"
#include "details/reporting/report.h"

using namespace Yuni;

namespace ny {

/*static*/ const Classdef Classdef::nullcdef;

template<class T, class TableT>
void Classdef::doPrint(T& out, const TableT& table) const {
	// qualifiers
	{
		bool cIsConst = qualifiers.constant;
		bool cIsRef   = qualifiers.ref;
		if (cIsConst or cIsRef) {
			if (cIsConst and cIsRef)
				out << "cref ";
			else
				out << ((cIsConst) ? "const " : "ref ");
		}
	}
	const Atom* selfAtom = table.findClassdefAtom(*this);
	if (selfAtom) {
		if (kind == CType::t_ptr)
			out << "ptr -> ";
		selfAtom->retrieveCaption(out, table);
	}
	else
		out << toString(kind);
	if (unlikely(qualifiers.nullable))
		out << '?';
}

void Classdef::print(Yuni::String& out, const ClassdefTableView& table, bool clearBefore) const {
	if (clearBefore)
		out.clear();
	doPrint(out, table);
}

void Classdef::print(Logs::Report& report, const ClassdefTableView& table) const {
	doPrint(report.data().message, table);
}

bool Classdef::isClass() const {
	return isLinkedToAtom() and (atom->type == Atom::Type::classdef);
}

} // namespace ny
