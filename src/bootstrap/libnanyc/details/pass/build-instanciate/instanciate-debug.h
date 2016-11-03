#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include "details/atom/classdef-table-view.h"
#include "details/ir/isa/data.h"
#include "details/ir/sequence.h"
#include "details/errors/errors.h"
#include "stack-frame.h"




namespace ny
{
namespace Pass
{
namespace Instanciate
{


	//! Print (to report) an IR sequence
	void debugPrintIRSequence(const YString& symbolName, const IR::Sequence&, const ClassdefTableView&, uint32_t offset = 0);

	//! Print the original opcodes sequence, produced from the AST
	void debugPrintSourceOpcodeSequence(const ClassdefTableView&, const Atom&, const char* usertxt);

	//! Print all type defintions (classdef) available in the current frame
	void debugPrintClassdefs(const AtomStackFrame&, const ClassdefTableView&);




} // namespace Instanciate
} // namespace Pass
} // namespace ny
