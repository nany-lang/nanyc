#include "intrinsics-nanyc-reflection.h"
#include "deprecated-error.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace semantic {
namespace reflect {


namespace {


template<class T>
bool reflect(Analyzer& analyzer, uint32_t lvid, nytype_t type, const T& callback) {
	uint32_t typelvid = analyzer.pushedparams.func.indexed[0].lvid;
	auto& cdeftable = analyzer.cdeftable;
	auto& frame = *analyzer.frame;
	auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, typelvid});
	cdeftable.substitute(lvid).mutateToBuiltinOrVoid(type);
	frame.lvids(lvid).synthetic = false;
	auto* atom = cdeftable.findClassdefAtom(cdef);
	if (unlikely(!atom))
		return complain::classRequired();
	if (analyzer.canGenerateCode())
		return callback(*atom);
	return true;
}


template<Atom::Type T>
inline bool is(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.type == T);
		return true;
	});
}


template<class T>
inline bool sum(Analyzer& analyzer, uint32_t lvid, const T& predicate) {
	return reflect(analyzer, lvid, nyt_u32, [&](const Atom& atom) {
		uint32_t count = 0;
		atom.eachChild([&](const Atom& child) -> bool {
			if (predicate(child))
				++count;
			return true;
		});
		ir::emit::constantu64(analyzer.out, lvid, count);
		return true;
	});
}


} // namespace


bool foreach(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_void, [&analyzer](Atom& atom) -> bool {
		auto& cdeftable = analyzer.cdeftable;
		// param 0 - type - see 'Atom& atom'
		// param 1 - callback
		uint32_t cbackLvid = analyzer.pushedparams.func.indexed[1].lvid;
		auto& cbackCdef = cdeftable.classdefFollowClassMember(CLID{analyzer.frame->atomid, cbackLvid});
		if (unlikely(cbackCdef.isBuiltinOrVoid()))
			return (error() << "object expected for parameter 2");
		auto* cbackAtom = cdeftable.findClassdefAtom(cbackCdef);
		if (unlikely(!cbackAtom))
			return (ice() << "cxx::reflect::foreach");
		auto count = atom.childrenCount();
		if (!count)
			return true;
		auto& reflectCall = *cdeftable.atoms().core.reflection.call;
		IndexedParameter paramTmpl{0, analyzer.currentLine, analyzer.currentOffset};
		IndexedParameter paramCallback{cbackLvid, analyzer.currentLine, analyzer.currentOffset};
		uint32_t lvid = analyzer.createLocalVariables(count * 2);
		atom.eachChild([&](Atom& child) -> bool {
			cdeftable.substitute(lvid).mutateToAtom(&child);
			paramTmpl.lvid = lvid++;
			analyzer.pushedparams.clear();
			analyzer.pushedparams.gentypes.indexed.emplace_back(paramTmpl);
			analyzer.pushedparams.func.indexed.emplace_back(paramCallback);
			//auto* typeinfo = analyzer.instanciateAtomFunc(reflectCall);
			return true;
		});
		return true;
	});
}


bool name(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(analyzer.out, lvid, atom.name());
		return true;
	});
}


bool classname(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(analyzer.out, lvid, atom.caption(analyzer.cdeftable));
		return true;
	});
}


bool keyword(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(analyzer.out, lvid, atom.keyword());
		return true;
	});
}


bool filename(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_ptr, [&](const Atom& atom) {
		ir::emit::constantText(analyzer.out, lvid, atom.origin.filename);
		return true;
	});
}


bool column(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_u32, [&](const Atom& atom) {
		ir::emit::constantu64(analyzer.out, lvid, atom.origin.offset);
		return true;
	});
}


bool line(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_u32, [&](const Atom& atom) {
		ir::emit::constantu64(analyzer.out, lvid, atom.origin.line);
		return true;
	});
}


bool isClass(Analyzer& analyzer, uint32_t lvid) {
	return is<Atom::Type::classdef>(analyzer, lvid);
}


bool isFunc(Analyzer& analyzer, uint32_t lvid) {
	return is<Atom::Type::funcdef>(analyzer, lvid);
}


bool isVar(Analyzer& analyzer, uint32_t lvid) {
	return is<Atom::Type::vardef>(analyzer, lvid);
}


bool isTypedef(Analyzer& analyzer, uint32_t lvid) {
	return is<Atom::Type::typealias>(analyzer, lvid);
}


bool isView(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.isView());
		return true;
	});
}


bool isOperator(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.isOperator());
		return true;
	});
}


bool ctor(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.isCtor());
		return true;
	});
}


bool dtor(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.isDtor());
		return true;
	});
}


bool callable(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.callable());
		return true;
	});
}


bool anonymous(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_bool, [&](const Atom& atom) {
		ir::emit::constantbool(analyzer.out, lvid, atom.isAnonymousClass());
		return true;
	});
}


bool bytes(Analyzer& analyzer, uint32_t lvid) {
	return reflect(analyzer, lvid, nyt_u32, [&](const Atom& atom) {
		ir::emit::constantu64(analyzer.out, lvid, static_cast<uint32_t>(atom.runtimeSizeof()));
		return true;
	});
}


} // namespace reflect
} // namespace semantic
} // namespace Nany


namespace ny {
namespace semantic {
namespace reflect {
namespace props {


bool count(Analyzer& analyzer, uint32_t lvid) {
	return sum(analyzer, lvid, [](const Atom& atom) -> bool {
		return atom.isProperty();
	});
}


} // namespace props
} // namespace reflect
} // namespace semantic
} // namespace ny


namespace ny {
namespace semantic {
namespace reflect {
namespace vars {


bool count(Analyzer& analyzer, uint32_t lvid) {
	return sum(analyzer, lvid, [](const Atom& atom) -> bool {
		return atom.isMemberVariable();
	});
}


} // namespace vars
} // namespace reflect
} // namespace semantic
} // namespace ny


namespace ny {
namespace semantic {
namespace reflect {
namespace funcs {


bool count(Analyzer& analyzer, uint32_t lvid) {
	return sum(analyzer, lvid, [](const Atom& atom) -> bool {
		return atom.isFunctor();
	});
}


} // namespace funcs
} // namespace reflect
} // namespace semantic
} // namespace ny
