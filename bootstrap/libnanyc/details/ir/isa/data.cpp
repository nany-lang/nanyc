#include "data.h"
#include "printer.inc.hpp"
#include "../sequence.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace isa {


String print(const Sequence& sequence, const ny::ir::Instruction& instr, const ny::AtomMap* atommap) {
	String text;
	Printer<String> printer{text, sequence};
	printer.atommap = atommap;
	printer.visit(instr);
	text.trimRight();
	return text;
}


void printExtract(YString& out, const Sequence& sequence, uint32_t offset, const AtomMap* atommap) {
	uint32_t context = 6;
	uint32_t start = (offset >= context) ? (offset - context) : 0;
	uint32_t end   = offset + 2;
	Printer<String> printer{out, sequence};
	printer.atommap = atommap;
	for (uint32_t i = start; i <= end and i < sequence.opcodeCount(); ++i) {
		if (i == offset)
			out << "   > | ";
		else
			out << "     | ";
		printer.visit(sequence.at(i));
	}
	out.trimRight();
}


} // namespace isa
} // namespace ir
} // namespace ny
