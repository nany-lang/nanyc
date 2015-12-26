#include "data.h"
#include "printer.inc.hpp"
#include "../program.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace ISA
{


	String print(const Program& program, const Nany::IR::Instruction& instr, const Nany::AtomMap* atommap)
	{
		String text;
		Printer<String> printer{text, program};
		printer.atommap = atommap;
		printer.visit(instr);
		text.trimRight();
		return text;
	}




} // namespace ISA
} // namespace IR
} // namespace Nany
