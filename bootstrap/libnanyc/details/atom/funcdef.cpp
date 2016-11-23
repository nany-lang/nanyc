#include "funcdef.h"

using namespace Yuni;


namespace ny {


void Funcdef::print(Yuni::String& out, bool clearBefore) const {
	if (clearBefore)
		out.clear();
	out << "attr ";
	if (not clid.isVoid())
		out << clid;
	else {
		if (not name.empty())
			out << "<unresolved>";
	}
	out << " as ";
	if (not name.empty())
		out << name;
	else
		out << "<self> ";
	if (not parameters.empty()) {
		out << '(';
		bool first = true;
		for (auto& pair : parameters) {
			if (not first)
				out << ", ";
			if (not pair.first.empty())
				out << pair.first << ": ";
			out << pair.second;
			first = false;
		}
		out << ')';
	}
	if (not rettype.isVoid())
		out << ": " << rettype;
	auto overloadCount = overloads.size();
	if (overloadCount > 0)
		out << " (<unresolved>, " << overloadCount << " overloads)";
	else {
		if (atom == nullptr and not name.empty())
			out << " (<unresolved>)";
	}
}


} // namespace ny
