#include "data.h"
#include "printer.inc.hpp"
#include "../sequence.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace isa {


String print(const Sequence& ircode, const ny::ir::Instruction& instr, const ny::AtomMap* atommap) {
	String text;
	Printer<String> printer{text, ircode};
	printer.atommap = atommap;
	printer.visit(instr);
	text.trimRight();
	return text;
}


void printExtract(YString& out, const Sequence& ircode, uint32_t offset, const AtomMap* atommap) {
	uint32_t context = 6;
	uint32_t start = (offset >= context) ? (offset - context) : 0;
	uint32_t end   = offset + 2;
	Printer<String> printer{out, ircode};
	printer.atommap = atommap;
	for (uint32_t i = start; i <= end and i < ircode.opcodeCount(); ++i) {
		if (i == offset)
			out << "   > | ";
		else
			out << "     | ";
		printer.visit(ircode.at(i));
	}
	out.trimRight();
}


} // namespace isa
} // namespace ir
} // namespace ny
