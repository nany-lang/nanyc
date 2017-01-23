#include "complain.h"
#include "details/errors/errors.h"
#include "details/ir/isa/data.h"
#include "exception.h"


namespace ny {
namespace complain {


void Error::complain() const {
	error() << msg;
}


void ICE::complain() const {
	ice() << msg;
}


Error::Error(const AnyString& msg)
	: msg{msg} {
}


Opcode::Opcode(const ny::ir::Sequence& ircode, const ir::Instruction& operands, const AnyString& usermsg) {
	// ICE: unknown opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
	if (not usermsg.empty())
		msg << usermsg << ':';
	else
		msg << "invalid opcode ";
	msg << " '" << ny::ir::isa::print(ircode, operands) << '\'';
}


} // namespace complain
} // namespace ny
