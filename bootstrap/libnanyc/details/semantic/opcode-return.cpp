#include "semantic-analysis.h"
#include "deprecated-error.h"
#include "ref-unref.h"

using namespace Yuni;

namespace ny::semantic {

namespace {

const Classdef* getExpectedReturnType(Atom& atom, ClassdefTableView& cdeftable) {
	if (not atom.returnType.clid.isVoid()) {
		auto* cdef = &cdeftable.classdefFollowClassMember(atom.returnType.clid);
		if (not cdef->isVoid())
			return cdef;
	}
	return nullptr;
}

const Classdef* getActualExprType(uint32_t lvid, uint32_t atomid, const ClassdefTableView& cdeftable) {
	if (lvid != 0) {
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{atomid, lvid});
		if (not cdef.isVoid())
			return &cdef;
	}
	return nullptr;
}

} // namespace

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::ret>& operands) {
	if (unlikely(frame->atom.type != Atom::Type::funcdef))
		return (void)(error() << "return values are only accepted in functions");
	auto* expectedType = getExpectedReturnType(frame->atom, cdeftable);
	auto* actualType = getActualExprType(operands.lvid, frame->atomid, cdeftable);
	// similarty between the two types to detect if an implicit convertion is required
	auto similarity = TypeCheck::Match::strictEqual;
	bool hasReturnValue = expectedType and actualType;
	if (hasReturnValue) {
		// determining if the expression returned matched the return type of the current func
		similarity = TypeCheck::isSimilarTo(*this, *actualType, *expectedType);
		switch (similarity) {
			case TypeCheck::Match::strictEqual: {
				break;
			}
			case TypeCheck::Match::equal: {
				if (expectedType->isAny()) // accept implicit convertions to 'any'
					break;
				return (void) complain::returnTypeImplicitConversion(*expectedType, *actualType);
			}
			case TypeCheck::Match::none: {
				return (void) complain::returnTypeMismatch(*expectedType, *actualType);
			}
		}
		// the return expr matches the return type requested by the source code
		// if this type is any, checking for previous return types
		if (expectedType->isAny() and not frame->returnValues.empty()) {
			auto& marker = frame->returnValues.front();
			if (not marker.clid.isVoid()) {
				auto& cdefPreviousReturn = cdeftable.classdef(marker.clid);
				auto prevsim = TypeCheck::isSimilarTo(*this, *actualType, cdefPreviousReturn);
				switch (prevsim) {
					case TypeCheck::Match::strictEqual: {
						break; // nice ! they share the same exact type !
					}
					case TypeCheck::Match::equal: {
						return (void) complain::returnTypeImplicitConversion(cdefPreviousReturn, *actualType, marker.line,
							marker.offset);
					}
					case TypeCheck::Match::none: {
						return (void) complain::returnMultipleTypes(cdefPreviousReturn, *actualType, marker.line, marker.offset);
					}
				}
			}
			else {
				// can not be void
				return (void) complain::returnTypeMissing(nullptr, actualType);
			}
		}
	}
	else {
		if (actualType == nullptr and expectedType == nullptr) {
			// both void
		}
		else {
			if (expectedType and expectedType->isAny()) {
				// accept the following:
				//    func foo: any { return; }
			}
			else {
				// one of the values are 'null'
				if (unlikely(actualType != nullptr or expectedType != nullptr))
					return (void) complain::returnTypeMissing(expectedType, actualType);
			}
		}
	}
	auto& spare = cdeftable.substitute(1);
	if (hasReturnValue) {
		spare.import(*actualType);
		if (not actualType->isBuiltinOrVoid())
			spare.mutateToAtom(cdeftable.findClassdefAtom(*actualType));
	}
	else {
		spare.mutateToVoid();
		spare.qualifiers.clear();
	}
	// the return valus is accepted
	frame->returnValues.emplace_back(ReturnValueMarker {
		(actualType ? actualType->clid : CLID{}), currentLine, currentOffset
	});
	if (unlikely(not canGenerateCode()))
		return;
	switch (similarity) {
		case TypeCheck::Match::equal: {
			// implicit convertion
			// not currently supported
			// - fallthru
		}
		case TypeCheck::Match::strictEqual: {
			ir::emit::trace(out, "return from func");
			if (hasReturnValue) {
				uint32_t retlvid;
				if (spare.qualifiers.ref or spare.isBuiltinOrVoid())
					retlvid = operands.lvid;
				else {
					// a copy seems necessary...
					retlvid = operands.tmplvid;
					// trying to copy the value first
					if (unlikely(not frame->verify(retlvid)))
						return frame->invalidate(retlvid);
					auto& spareTmp = cdeftable.substitute(operands.tmplvid);
					spareTmp.import(spare);
					spareTmp.qualifiers.merge(spare.qualifiers);
					frame->lvids(operands.tmplvid).synthetic = false;
					bool r = instanciateAssignment(*frame, retlvid, operands.lvid, false);
					if (unlikely(not r))
						return frame->invalidate(retlvid);
				}
				tryToAcquireObject(*this, retlvid, spare);
				releaseAllScopedVariables();
				ir::emit::ret(out, retlvid, 0);
			}
			else {
				releaseAllScopedVariables();
				ir::emit::ret(out);
			}
			break;
		}
		case TypeCheck::Match::none: {
		}
	}
}

} // ny::semantic
