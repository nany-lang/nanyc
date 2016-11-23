#include "classdef-overloads.h"
#include "details/atom/atom.h"

using namespace Yuni;


namespace ny {

void ClassdefOverloads::print(String& out, const ClassdefTableView& table, bool clearBefore) const {
	if (clearBefore)
		out.clear();
	for (auto& atom : m_overloads) {
		out << "        overload ";
		atom.get().retrieveCaption(out, table);
		out << '\n';
	}
}


} // namespace ny
