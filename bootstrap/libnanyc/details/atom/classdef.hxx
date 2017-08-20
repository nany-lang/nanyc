#pragma once
#include "classdef.h"

namespace ny {

inline Classdef::Classdef()
	: qualifiers() {
}

inline Classdef::Classdef(const CLID& clid)
	: kind(CType::t_any)
	, clid(clid) {
}

inline bool Classdef::isBuiltin() const {
	return (kind != CType::t_void) and (kind != CType::t_any);
}

inline bool Classdef::isBuiltinOrVoid() const {
	return kind != CType::t_any;
}

inline bool Classdef::isRawPointer() const {
	return kind == CType::t_ptr and (atom == nullptr);
}

inline bool Classdef::isBuiltinU64() const {
	return kind == CType::t_u64;
}

inline bool Classdef::isBuiltinU32() const {
	return kind == CType::t_u32;
}

inline bool Classdef::isBuiltinU8() const {
	return kind == CType::t_u8;
}

inline bool Classdef::isBuiltingUnsigned() const {
	switch (kind) {
		case CType::t_u8:
		case CType::t_u16:
		case CType::t_u32:
		case CType::t_u64:
			return true;
		default:
			return false;
	}
}

inline bool Classdef::isVoid() const {
	return kind == CType::t_void;
}

inline bool Classdef::isLinkedToAtom() const {
	assert((nullptr == atom or (kind == CType::t_any))
		   and "the kind of a classdef must be 'any' if an atom is provided");
	return nullptr != atom;
}

inline bool Classdef::isAny() const {
	return (nullptr == atom) and (kind == CType::t_any);
}

inline bool Classdef::hasAtom() const {
	return nullptr != atom;
}

inline bool Classdef::isVariable() const {
	return instance;
}

inline void Classdef::mutateToVoid() {
	kind = CType::t_void;
	atom = nullptr;
}

inline void Classdef::mutateToBuiltin(CType newkind) {
	assert(newkind != CType::t_void and newkind != CType::t_any);
	kind = newkind;
	atom = nullptr;
}

inline void Classdef::mutateToBuiltinOrVoid(CType newkind) {
	assert(newkind != CType::t_any);
	kind = newkind;
	atom = nullptr;
}

inline void Classdef::mutateToAny() {
	kind = CType::t_any;
	atom = nullptr;
}

inline void Classdef::mutateToAtom(Atom* newAtom) {
	kind = CType::t_any;
	atom = newAtom;
}

inline void Classdef::mutateToPtr2Func(Atom* newAtom) {
	kind = CType::t_ptr;
	atom = newAtom;
}

inline bool Classdef::hasConstraints() const {
	return not isAny() or not interface.empty() or not followup.empty(); // TODO qualifiers ???
}

inline void Classdef::import(const Classdef& rhs) {
	kind = rhs.kind;
	atom = rhs.atom;
	parentclid = rhs.parentclid;
	instance   = rhs.instance;
	interface  = rhs.interface;  // mandatory to preserve constraints
	followup   = rhs.followup;   // same here
	origins    = rhs.origins;
	// qualifiers should be preserved
	// qualifiers = rhs.qualifiers;
}

inline YString Classdef::print(const ClassdefTableView& table) const {
	YString out;
	print(out, table);
	return out;
}

} // namespace ny
