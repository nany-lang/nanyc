#include "data.h"
#include "printer.inc.hpp"
#include "../sequence.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace ISA
{


	String print(const Sequence& sequence, const Nany::IR::Instruction& instr, const Nany::AtomMap* atommap)
	{
		String text;
		Printer<String> printer{text, sequence};
		printer.atommap = atommap;
		printer.visit(instr);
		text.trimRight();
		return text;
	}




} // namespace ISA
} // namespace IR
} // namespace Nany
