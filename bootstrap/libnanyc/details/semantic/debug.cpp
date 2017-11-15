#include "debug.h"

using namespace Yuni;

namespace ny {
namespace semantic {

void debugPrintIRSequence(const String& symbolName, const ir::Sequence& irseq,
						  const ClassdefTableView& table, uint32_t offset) {
	String text;
	text.reserve(irseq.opcodeCount() * 40); // arbitrary
	irseq.print(text, &table.atoms(), offset);
	text.replace("\n", "\n    ");
	text.trimRight();
	info();
	auto entry = trace();
	entry.message.prefix << symbolName;
	entry.trace() << "{\n    " << text << "\n}";
	entry.info();
	entry.info();
	entry.info(); // for beauty
}

void debugPrintSourceOpcodeSequence(const ClassdefTableView& table, const Atom& atom, const char* usertxt) {
	String text;
	text << usertxt << table.keyword(atom) << ' '; // ex: func
	atom.retrieveCaption(text, table);  // ex: A.foo(...)...
	uint32_t offset = atom.opcodes.offset;
	debugPrintIRSequence(text, *atom.opcodes.ircode, table, offset);
}

void debugPrintClassdefs(const AtomStackFrame& frame, const ClassdefTableView& table) {
	// new entry
	{
		auto entry = trace();
		entry.message.prefix << table.keyword(frame.atom) << ' ';
		frame.atom.retrieveCaption(entry.message.prefix, table);
		entry << " - type matrix, after instanciation - atom " << frame.atom.atomid;
	}
	for (uint32_t i = 0; i != frame.localVariablesCount(); ++i) {
		auto clid = CLID{frame.atomid, i};
		auto entry = trace();
		if (table.hasClassdef(clid) or table.hasSubstitute(clid)) {
			table.printClassdef(entry.message.message, clid, table.classdef(clid));
			entry.message.message.trimRight();
			if (table.hasSubstitute(clid))
				entry << " (local replacement)";
			if (frame.lvids(i).isConstexpr)
				entry << " (constexpr)";
			if (frame.lvids(i).errorReported)
				entry << " [ERROR]";
		}
		else
			entry << "    " << clid << ": !!INVALID CLID";
	}
}

} // namespace semantic
} // namespace ny
