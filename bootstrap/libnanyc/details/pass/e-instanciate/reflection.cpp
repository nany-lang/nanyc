#include "reflection.h"
#include "instanciate-error.h"
#include "details/ir/emit.h"

using namespace Yuni;
using namespace ny::Pass::Instanciate;


namespace ny {
namespace reflect {


namespace {


template<class T>
bool reflect(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid, nytype_t type, const T& callback) {
	uint32_t typelvid = seq.pushedparams.func.indexed[0].lvid;
	auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, typelvid});
	seq.cdeftable.substitute(lvid).mutateToBuiltinOrVoid(type);
	seq.frame->lvids(lvid).synthetic = false;
	auto* atom = seq.cdeftable.findClassdefAtom(cdef);
	if (unlikely(!atom))
		return complain::classRequired();
	if (seq.canGenerateCode())
		callback(*atom);
	return true;
}


template<Atom::Type T>
inline bool is(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.type == T);
	});
}


template<class T>
inline bool sum(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid, const T& predicate) {
	return reflect(seq, lvid, nyt_u32, [&](const Atom& atom) {
		uint32_t count = 0;
		atom.eachChild([&](const Atom& child) -> bool {
			if (predicate(child))
				++count;
			return true;
		});
		ir::emit::constantu64(seq.out, lvid, count);
	});
}


} // namespace


bool name(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(seq.out, lvid, atom.name());
	});
}


bool classname(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(seq.out, lvid, atom.caption(seq.cdeftable));
	});
}


bool keyword(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(seq.out, lvid, atom.keyword());
	});
}


bool filename(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(seq.out, lvid, atom.origin.filename);
	});
}


bool column(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_u32, [&](const Atom& atom) {
		ir::emit::constantu64(seq.out, lvid, atom.origin.offset);
	});
}


bool line(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_u32, [&](const Atom& atom) {
		ir::emit::constantu64(seq.out, lvid, atom.origin.line);
	});
}


bool isClass(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return is<Atom::Type::classdef>(seq, lvid);
}


bool isFunc(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return is<Atom::Type::funcdef>(seq, lvid);
}


bool isVar(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return is<Atom::Type::vardef>(seq, lvid);
}


bool isTypedef(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return is<Atom::Type::typealias>(seq, lvid);
}


bool isView(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.isView());
	});
}


bool isOperator(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.isOperator());
	});
}


bool ctor(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.isCtor());
	});
}


bool dtor(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.isDtor());
	});
}


bool callable(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.callable());
	});
}


bool anonymous(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(seq.out, lvid, atom.isAnonymousClass());
	});
}


bool bytes(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_u32, [&](const Atom& atom) {
		ir::emit::constantu64(seq.out, lvid, static_cast<uint32_t>(atom.runtimeSizeof()));
	});
}


bool begin(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return reflect(seq, lvid, nyt_void, [&](Atom& atom) {
		seq.reflect.atom = &atom;
	});
}


bool item(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	auto& retval = seq.cdeftable.substitute(lvid);
	retval.mutateToVoid();
	assert(seq.reflect.atom);
	seq.reflect.atom->eachChild([&](Atom& child) -> bool {
		retval.mutateToAtom(&child);
		return false;
	});
	return true;
}


bool end(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	seq.cdeftable.substitute(lvid).mutateToVoid();
	seq.reflect.atom = nullptr;
	return true;
}


} // namespace reflect
} // namespace Nany


namespace ny {
namespace reflect {
namespace props {


bool count(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return sum(seq, lvid, [](const Atom& atom) -> bool {
		return atom.isProperty();
	});
}


} // namespace props
} // namespace reflect
} // namespace ny


namespace ny {
namespace reflect {
namespace vars {


bool count(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return sum(seq, lvid, [](const Atom& atom) -> bool {
		return atom.isMemberVariable();
	});
}


} // namespace vars
} // namespace reflect
} // namespace ny


namespace ny {
namespace reflect {
namespace funcs {


bool count(ny::Pass::Instanciate::SequenceBuilder& seq, uint32_t lvid) {
	return sum(seq, lvid, [](const Atom& atom) -> bool {
		return atom.isFunctor();
	});
}


} // namespace funcs
} // namespace reflect
} // namespace ny
