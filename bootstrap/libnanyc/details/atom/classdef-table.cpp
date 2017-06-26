#include "classdef-table.h"
#include "classdef-table-view.h"
#include <yuni/core/tribool.h>
#include <cassert>

using namespace Yuni;


namespace ny {


ClassdefTable::ClassdefTable()
	: atoms(stringrefs) {
	m_classdefs.insert(std::make_pair(CLID{}, new Classdef));
}


bool ClassdefTable::makeHardlink(const CLID& source, const CLID& target) {
	assert(not source.isVoid() and "invalid source CLID");
	assert(not target.isVoid() and "invalid target CLID");
	assert(source != target and "same target !");
	// looking for the source
	auto it = m_classdefs.find(source);
	if (it != m_classdefs.end()) {
		// return *(it->second);
		// looking for the target
		auto ittarget = m_classdefs.find(target);
		if (ittarget != m_classdefs.end()) {
			// the target has been found, replacing the classdef
			ittarget->second = it->second;
			return true;
		}
		//else
		//{
		//  // the target has not been found
		//  m_classdefs.insert(std::make_pair(target, it->second));
		//}
		//return true;
	}
	return false;
}


Classdef& ClassdefTable::classdef(const CLID& clid) {
	assert(not clid.isVoid() and "invalid clid");
	// pick first substitutes
	if (clid.atomid() == m_layer.atomid) {
		assert(clid.lvid() < m_layer.count);
		if (m_layer.flags[clid.lvid()])
			return m_layer.storage[clid.lvid()];
	}
	auto it = m_classdefs.find(clid);
	if (unlikely(it == m_classdefs.end())) {
		assert(false and "classdef not found");
		it = m_classdefs.find(CLID{});
	}
	auto& result = *(it->second);
	if (result.clid.atomid() == m_layer.atomid) { // dealing with hard links
		auto lvid = result.clid.lvid();
		assert(lvid < m_layer.count);
		if (m_layer.flags[lvid])
			return m_layer.storage[lvid];
	}
	return result;
}


const Classdef& ClassdefTable::rawclassdef(const CLID& clid) const {
	assert(not clid.isVoid() and "invalid clid");
	// TODO use an alternate (and more efficient) container for the special classdefs atomid=0
	auto it = m_classdefs.find(clid);
	if (unlikely(it == m_classdefs.end())) {
		assert(false and "failed to find clid");
		it = m_classdefs.find(CLID{});
	}
	return *(it->second);
}


Classdef& ClassdefTable::rawclassdef(const CLID& clid) {
	assert(not clid.isVoid() and "invalid clid");
	// TODO use an alternate (and more efficient) container for the special classdefs atomid=0
	auto it = m_classdefs.find(clid);
	if (unlikely(it == m_classdefs.end())) {
		assert(false and "failed to find clid");
		it = m_classdefs.find(CLID{});
	}
	return *(it->second);
}


const Classdef& ClassdefTable::classdef(const CLID& clid) const {
	assert(not clid.isVoid() and "invalid clid");
	// TODO use an alternate (and more efficient) container for the special classdefs atomid=0
	// pick first substitutes
	if (clid.atomid() == m_layer.atomid) {
		assert(clid.lvid() < m_layer.count);
		if (m_layer.flags[clid.lvid()])
			return m_layer.storage[clid.lvid()];
	}
	auto it = m_classdefs.find(clid);
	if (unlikely(it == m_classdefs.end())) {
		assert(false and "classdef not found");
		it = m_classdefs.find(CLID{});
	}
	auto& result = *(it->second);
	if (result.clid.atomid() == m_layer.atomid) { // dealing with hard links
		auto lvid = result.clid.lvid();
		assert(lvid < m_layer.count);
		if (m_layer.flags[lvid])
			return m_layer.storage[lvid];
	}
	return result;
}


const Classdef& ClassdefTable::classdefFollowClassMember(const CLID& clid) const {
	auto& cdef = classdef(clid);
	auto* atom = findClassdefAtom(cdef);
	if (atom and atom->isMemberVariable())
		return classdef(atom->returnType.clid);
	return cdef;
}


Funcdef& ClassdefTable::addClassdefInterface(const CLID& clid, const AnyString& name) {
	assert(not clid.isVoid() and "invalid clid");
	// empty name is allowed here (for representing the parent atom)
	auto* funcdef = (name.empty()) ? new Funcdef(nullptr) : new Funcdef(stringrefs.refstr(name));
	auto& ref = classdef(clid);
	ref.interface.add(funcdef);
	return *funcdef;
}


Funcdef& ClassdefTable::addClassdefInterfaceSelf(const CLID& clid, const AnyString& name) {
	assert(not clid.isVoid() and "invalid clid");
	// empty name is allowed here (for representing the parent atom)
	auto* funcdef = (name.empty()) ? new Funcdef(nullptr) : new Funcdef(stringrefs.refstr(name));
	classdef(clid).interface.pSelf = funcdef;
	return *funcdef;
}


void ClassdefTable::bulkCreate(std::vector<CLID>& out, uint32_t atomid, uint32_t count) {
	assert(atomid > 0);
	++count; // 1-based
	out.clear();
	out.reserve(count);
	out.push_back(CLID{});
	for (uint32_t i = 1; i != count; ++i) {
		// the new classid, made from the atom id
		CLID clid{atomid, i};
		out.push_back(clid);
		// check that the entry does not already exists
		assert(m_classdefs.find(clid) == m_classdefs.end());
		// insert the new classdef
		m_classdefs.insert(std::make_pair(clid, new Classdef{clid}));
	}
}


void ClassdefTable::bulkAppend(uint32_t atomid, uint32_t offset, uint32_t count) {
	assert(atomid > 0);
	for (uint32_t i = offset; i != offset + count; ++i) {
		// the new classid, made from the atom id
		CLID clid{atomid, i};
		// check that the entry does not already exists
		assert(m_classdefs.find(clid) == m_classdefs.end());
		// insert the new classdef
		m_classdefs.insert(std::make_pair(clid, new Classdef{clid}));
	}
}


void ClassdefTable::registerAtom(Atom& atom) {
	// TODO use an alternate (and more efficient) container for this special classdefs
	const CLID& clid = CLID::AtomMapID(atom.atomid);
	auto* newClassdef = new Classdef(clid);
	newClassdef->mutateToAtom(&atom);
	m_classdefs.insert(std::make_pair(clid, newClassdef));
}


Atom* ClassdefTable::findRawClassdefAtom(const Classdef& cdef) const {
	Atom* result = cdef.hasAtom() ? cdef.atom : nullptr;
	if (nullptr == result and not cdef.followup.extends.empty()) {
		Atom* followAtom = nullptr;
		for (auto& clid : cdef.followup.extends) {
			auto& followup = rawclassdef(clid);
			if (followup.hasAtom()) {
				if (unlikely(followAtom != nullptr))
					return nullptr;
				followAtom = followup.atom;
			}
		}
		result = followAtom;
	}
	return result;
}


Atom* ClassdefTable::findClassdefAtom(const Classdef& cdef) const {
	Atom* result = cdef.hasAtom() ? cdef.atom : nullptr;
	if (nullptr == result and not cdef.followup.extends.empty()) {
		Atom* followAtom = nullptr;
		for (auto& clid : cdef.followup.extends) {
			auto& followup = classdef(clid);
			if (followup.hasAtom()) {
				if (unlikely(followAtom != nullptr))
					return nullptr;
				followAtom = followup.atom;
			}
		}
		result = followAtom;
	}
	return result;
}


bool ClassdefTable::hasSubstitute(CLID clid) const {
	if (hasClassdef(clid))
		clid = classdef(clid).clid; // take into consideration symlinks
	if (clid.atomid() == m_layer.atomid) {
		assert(clid.lvid() < m_layer.count);
		return (m_layer.flags[clid.lvid()]);
	}
	return false;
}


void ClassdefTable::substituteResize(uint32_t count) {
	uint32_t previous = m_layer.count;
	if (count <= previous)
		return;
	m_layer.count = count;
	m_layer.flags.resize(count);
	for (uint32_t i = previous; i != count; ++i)
		m_layer.flags[i] = false; // true;
	m_layer.storage.reserve(count);
	assert(m_layer.storage.size() <= count);
	for (uint32_t i = static_cast<uint32_t>(m_layer.storage.size()); i != count; ++i)
		m_layer.storage.emplace_back(CLID{m_layer.atomid, i});
	// checking for missing classdef
	// functions are sometimes generating on the fly (ctor, clone...)
	for (uint32_t i = previous; i != count; ++i) {
		CLID clid{m_layer.atomid, i};
		if (0 == m_classdefs.count(clid))
			m_classdefs[clid] = new Classdef(clid); // any with local replacement
	}
}


void ClassdefTable::mergeSubstitutes() {
	auto atomid = m_layer.atomid;
	if (unlikely(atomid == (uint32_t) - 1))
		throw "invalid atom id for merging substitutions";
	for (uint32_t i = 0; i != m_layer.count; ++i) {
		if (m_layer.flags[i])
			m_classdefs[CLID{atomid, i}] = new Classdef(m_layer.storage[i]);
	}
	// invalidate the current layer
	m_layer.atomid = static_cast<uint32_t>(-1);
}


Classdef& ClassdefTable::substitute(uint32_t lvid) const {
	assert(lvid < m_layer.count);
	if (not m_layer.flags[lvid]) {
		m_layer.flags[lvid] = true;
		auto& newcdef = m_layer.storage[lvid];
		// preserve qualifiers
		auto it = m_classdefs.find(CLID{m_layer.atomid, lvid});
		if (it != m_classdefs.end())
			newcdef.qualifiers = (*(it->second)).qualifiers;
		// set clid
		newcdef.clid.reclass(m_layer.atomid, lvid);
		return newcdef;
	}
	return m_layer.storage[lvid];
}


Classdef& ClassdefTable::addSubstitute(CType kind, Atom* atom, const Qualifiers& qualifiers) const {
	// atom can be null
	m_layer.flags.push_back(true);
	m_layer.storage.emplace_back();
	++m_layer.count;
	assert(m_layer.count == m_layer.flags.size());
	assert(m_layer.count == m_layer.storage.size());
	auto& ret = m_layer.storage.back();
	switch (kind) {
		case CType::t_any:
			ret.mutateToAtom(atom);
			break;
		default:
			ret.mutateToBuiltinOrVoid(kind);
	}
	ret.qualifiers = qualifiers; // preserve qualifiers
	ret.clid.reclass(m_layer.atomid, m_layer.count - 1);
	return ret;
}


AnyString ClassdefTable::keyword(const Atom& atom) const {
	switch (atom.type) {
		default:
			return atom.keyword();
		// for vardef, something more subtle can be done
		case Atom::Type::vardef: {
			// the clid might be invalid in development and this function is used for debugging
			// however, to indicate an issue, no valid identifier will be returned
			if (likely(not atom.returnType.clid.isVoid())) {
				if (hasClassdef(atom.returnType.clid)) { // to be fault-tolerant
					auto& qualifiers = classdef(atom.returnType.clid).qualifiers;
					if (qualifiers.constant) {
						if (qualifiers.ref)
							return "cref";
						return "const";
					}
					if (qualifiers.ref)
						return "ref";
					return "var";
				}
				return "var{invalid-clid}";
			}
			return "var[no-return-type]";
		}
	}
	return "auto";
}


} // namespace ny
