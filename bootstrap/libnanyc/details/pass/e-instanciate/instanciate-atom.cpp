#include "instanciate.h"
#include "details/reporting/report.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "details/pass/d-object-map/mapping.h"
#include "libnanyc-traces.h"
#include "instanciate-atom.h"
#include "instanciate-debug.h"
#include "instanciate-error.h"
#include <memory>

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


namespace {


void prepareSignature(Signature& signature, InstanciateData& info) {
	uint32_t count = static_cast<uint32_t>(info.params.size());
	if (count != 0) {
		signature.parameters.resize(count);
		for (uint32_t i = 0; i != count; ++i) {
			assert(info.params[i].cdef != nullptr);
			auto& cdef  = *(info.params[i].cdef);
			auto& param = signature.parameters[i];
			param.kind = cdef.kind;
			param.qualifiers = cdef.qualifiers;
			if (param.kind == nyt_any)
				param.atom = const_cast<Atom*>(info.cdeftable.findClassdefAtom(cdef));
			assert(param.kind != nyt_any or param.atom != nullptr);
		}
	}
	count = static_cast<uint32_t>(info.tmplparams.size());
	if (count != 0) {
		signature.tmplparams.resize(count);
		for (uint32_t i = 0; i != count; ++i) {
			assert(info.tmplparams[i].cdef != nullptr);
			auto& cdef  = *(info.tmplparams[i].cdef);
			auto& param = signature.tmplparams[i];
			param.atom = const_cast<Atom*>(info.cdeftable.findClassdefAtom(cdef));
			param.kind = cdef.kind;
			param.qualifiers = cdef.qualifiers;
			assert(param.kind != nyt_any or param.atom != nullptr);
		}
	}
}


bool makeNewAtomInstanciation(InstanciateData& info, Atom& atom) {
	// create a new atom with non-generic parameters / from a contextual atom
	// (generic/anonymous class)
	// re-map from the parent
	{
		auto& sequence  = *atom.opcodes.sequence;
		auto& originaltable = info.cdeftable.originalTable();
		// the mutex is useless here since the code instanciation is mono-threaded
		// but it's required by the mapping (which must be thread-safe in the first passes - see attach)
		// TODO remove this mutex
		Mutex mutex;
		Pass::Mapping::SequenceMapping mapper{originaltable, mutex, sequence};
		mapper.evaluateWholeSequence = false;
		mapper.prefixNameForFirstAtomCreated = "^";
		// run the type mapping
		mapper.map(*atom.parent, atom.opcodes.offset);
		if (unlikely(!mapper.firstAtomCreated))
			return (error() << "failed to remap atom '" << atom.caption() << "'");
		assert(info.atom.get().atomid != mapper.firstAtomCreated->atomid);
		assert(&info.atom.get() != mapper.firstAtomCreated);
		info.atom = std::ref(*mapper.firstAtomCreated);
	}
	// the generic parameters are fully resolved now, avoid
	// any new attempt to check them
	auto& newAtom = info.atom.get();
	newAtom.tmplparamsForPrinting.swap(newAtom.tmplparams);
	// marking the new atom as instanciated, like any standard atom
	newAtom.classinfo.isInstanciated = true;
	// to keep the types for the new atoms
	info.shouldMergeLayer = true;
	// upate parameter types
	bool r = resolveStrictParameterTypes(info.build, newAtom, &info);
	warning() << ".. instanciate from '" << atom.caption(info.cdeftable) << "' to '"
			  << newAtom.caption(info.cdeftable) << '\'';
	return r;
}


//! Prepare the first local registers according the given signature
void pushParameterTypes(SequenceBuilder& seq, Atom& atom, const Signature& signature) {
	assert(seq.frame == NULL);
	// magic constant +2
	//  * +1: all clid are 1-based (0 is reserved for the atom itself, not for an internal var)
	//  * +1: the CLID{X, 1} is reserved for the return type
	auto& cdeftable = seq.cdeftable;
	uint32_t count = signature.parameters.size();
	// unused pseudo/invalid register
	cdeftable.addSubstitute(nyt_void, nullptr, Qualifiers()); // unused, since 1-based
	// redefine return type {atomid,1}
	auto& rettype = cdeftable.rawclassdef(CLID{atom.atomid, 1});
	assert(atom.atomid == rettype.clid.atomid());
	Atom* atomparam = (not rettype.isBuiltinOrVoid()) ? cdeftable.findRawClassdefAtom(rettype) : nullptr;
	cdeftable.addSubstitute(rettype.kind, atomparam, rettype.qualifiers);
	auto substitute = [&](auto & parameter) {
		cdeftable.addSubstitute(parameter.kind, parameter.atom, parameter.qualifiers);
		assert(parameter.kind != nyt_any or parameter.atom != nullptr);
	};
	// parameters, as expected
	for (uint32_t i = 0; i != count; ++i)
		substitute(signature.parameters[i]);
	// reserved variables for cloning parameters (after normal parameters)
	for (uint32_t i = 0; i != count; ++i)
		substitute(signature.parameters[i]);
	// template parameters
	count = signature.tmplparams.size();
	for (uint32_t i = 0; i != count; ++i)
		substitute(signature.tmplparams[i]);
}


ir::Sequence* performAtomInstanciation(InstanciateData& info, Signature& signature) {
	auto& atomRequested = info.atom.get();
	if (unlikely(!atomRequested.opcodes.sequence or !atomRequested.parent)) {
		ice() << "invalid atom";
		return nullptr;
	}
	// In case or an anonymous class or a class with generic type parameters, it is
	// necessary to use new atoms (t needs a forked version to work on to have different types)
	if (atomRequested.isContextual()) {
		if (not makeNewAtomInstanciation(info, atomRequested))
			return nullptr;
		assert(&info.atom.get() != &atomRequested and "a new atom must be used");
	}
	auto& atom = info.atom.get(); // current atom, can be different from `atomRequested`
	if (!!atom.candidatesForCapture and info.parent)
		info.parent->captureVariables(atom);
	Atom::FlagAutoSwitch<Atom::Flags::instanciating> flagUpdater{atom};
	// registering the new instanciation first (required for recursive functions & classes)
	// `atomRequested` is probably `atom` itself, but different for template classes
	auto instance = atomRequested.instances.create(signature, &atom);
	info.instanceid = instance.id();
	// the original ir sequence generated from the AST
	auto& inputIR = *(atom.opcodes.sequence);
	// the new ir sequence for the instanciated function
	auto& outIR = instance.sequence();
	// new layer for the cdeftable
	ClassdefTableView newView{info.cdeftable, atom.atomid, signature.parameters.size()};
	// Error reporting
	Logs::Report report{*info.report};
	// instanciate the sequence attached to the atom
	auto builder = std::make_unique<SequenceBuilder>
				   (report.subgroup(), newView, info.build, &outIR, inputIR, info.parent);
	if (Config::Traces::sourceOpcodeSequence)
		debugPrintSourceOpcodeSequence(info.cdeftable, info.atom.get(), "[ir-from-ast] ");
	// transfert input parameters
	pushParameterTypes(*builder, atom, signature);
	builder->layerDepthLimit = 2; // allow the first blueprint to be instanciated
	// atomid mapping, usefull to keep track of the good atom id
	builder->mappingBlueprintAtomID[0] = atomRequested.atomid; // {from}
	builder->mappingBlueprintAtomID[1] = atom.atomid;          // {to}
	// Read the input ir sequence, resolve all types, and generate
	// a new ir sequence ready for execution ! (with or without optimization passes)
	// (everything happens here)
	bool success = builder->readAndInstanciate(atom.opcodes.offset);
	updateTypesInAllStackallocOp(outIR, newView, atom.atomid);
	// keep all deduced types
	if (/*likely(success) and*/ info.shouldMergeLayer)
		newView.mergeSubstitutes();
	// Generating the full human readable name of the symbol with the
	// apropriate types & qualifiers for all parameters
	// (example: "func A.foo(b: cref __i32): ref __i32")
	// note: the content of the string will be moved to avoid memory allocation
	String symbolName;
	if (success or Config::Traces::generatedOpcodeSequence) {
		symbolName << newView.keyword(info.atom) << ' '; // ex: func
		atom.retrieveCaption(symbolName, newView);  // ex: A.foo(...)...
	}
	if (Config::Traces::generatedOpcodeSequence)
		debugPrintIRSequence(symbolName, outIR, newView);
	if (success) {
		switch (atom.type) {
			case Atom::Type::funcdef:
			case Atom::Type::typealias: {
				// copying the 'returned' type to 'info'
				auto& cdefReturn = newView.classdef(CLID{atom.atomid, 1});
				if (not cdefReturn.isBuiltinOrVoid()) {
					auto* atom = newView.findClassdefAtom(cdefReturn);
					if (atom)
						info.returnType.mutateToAtom(atom);
					else
						success = (ice() << "invalid atom pointer in func return type for '" << symbolName << '\'');
				}
				else
					info.returnType.kind = cdefReturn.kind;
				break;
			}
			case Atom::Type::classdef: {
				// provides the default ctor implementation if not user-defined
				if (not atom.hasMember("^new"))
					atom.renameChild("^default-new", "^new");
			}
			// [[fallthru]]
			default: {
				// not a function, so no return value (unlikely to be used anyway)
				info.returnType.mutateToVoid();
			}
		}
		if (likely(success)) {
			instance.update(std::move(symbolName), info.returnType);
			return &outIR;
		}
	}
	// failed to instanciate the input ir sequence. This can be expected, if trying
	// to not instanciate the appropriate function (if several overloads are present for example)
	info.instanceid = instance.invalidate(signature);
	return nullptr;
}


bool instanciateRecursiveAtom(InstanciateData& info) {
	Atom& atom = info.atom.get();
	// mark the func as recursive
	atom.flags += Atom::Flags::recursive;
	if (unlikely(not atom.isFunction())) {
		ice() << "cannot mark non function '" << atom.caption() << "' as recursive";
		return false;
	}
	bool success = (info.parent
					and info.parent->getReturnTypeForRecursiveFunc(atom, info.returnType));
	if (unlikely(not success)) {
		info.returnType.mutateToAny();
		error() << "parameters/return types must be fully defined to allow recursive func calls";
		return false;
	}
	return true;
}


} // anonymous namespace


bool resolveStrictParameterTypes(Build& build, Atom& atom, InstanciateData* originalInfo) {
	switch (atom.type) {
		case Atom::Type::funcdef:
		case Atom::Type::typealias: {
			// this pass intends to resolve the given types for parameters
			// (to be able to deduce overload later).
			// or typealias for the same reason (they can used by parameters)
			bool needPartialInstanciation = not atom.parameters.empty() or atom.isTypeAlias();
			if (not needPartialInstanciation)
				break;
			// Classdef table layer
			ClassdefTableView cdeftblView{build.cdeftable};
			if (not (originalInfo and atom.isTypeAlias())) {
				// input parameters (won't be used)
				decltype(Pass::Instanciate::FuncOverloadMatch::result.params) params;
				decltype(Pass::Instanciate::FuncOverloadMatch::result.params) tmplparams;
				Logs::Message::Ptr newReport;
				// error reporting
				ny::Logs::Report report{*build.messages.get()};
				Pass::Instanciate::InstanciateData info {
					newReport, atom, cdeftblView, build, params, tmplparams
				};
				bool success = Pass::Instanciate::instanciateAtomParameterTypes(info);
				if (not success)
					report.appendEntry(newReport);
				return success;
			}
			else {
				// import generic type parameter from instanciation info
				auto& tmplparams = originalInfo->tmplparams;
				auto pindex = atom.classinfo.nextFieldIndex;
				if (unlikely(not (pindex < tmplparams.size())))
					return (ice() << "gen type invalid index");
				atom.returnType.clid = CLID::AtomMapID(atom.atomid);
				auto& srccdef = build.cdeftable.classdef(tmplparams[pindex].clid);
				auto& rawcdef = build.cdeftable.rawclassdef(CLID::AtomMapID(atom.atomid));
				rawcdef.import(srccdef);
				rawcdef.qualifiers.merge(srccdef.qualifiers);
				return true;
			}
			break;
		}
		case Atom::Type::classdef: {
			// do not try to do something on generic classes. It will be
			// done later when the generic param types will be available
			// (will be empty when instanciating a generic class)
			if (not atom.tmplparams.empty())
				return true;
		}
		// [[fallthru]]
		case Atom::Type::namespacedef:
			break;
		case Atom::Type::unit:
		case Atom::Type::vardef:
			return true;
	}
	bool success = true;
	// try to resolve all typedefs first
	atom.eachChild([&](Atom & subatom) -> bool {
		if (subatom.isTypeAlias())
			success &= resolveStrictParameterTypes(build, subatom, originalInfo);
		return true;
	});
	// .. everything else then
	atom.eachChild([&](Atom & subatom) -> bool {
		if (not subatom.isTypeAlias())
			success &= resolveStrictParameterTypes(build, subatom);
		return true;
	});
	return success;
}


bool SequenceBuilder::instanciateAtomClassClone(Atom& atom, uint32_t lvid, uint32_t rhs) {
	assert(not signatureOnly);
	assert(atom.isClass());
	if (unlikely(atom.flags(Atom::Flags::error)))
		return false;
	// first, try to find the user-defined clone function (if any)
	Atom* userDefinedClone = nullptr;
	switch (atom.findFuncAtom(userDefinedClone, "^clone")) {
		case 0:
			break;
		case 1: {
			assert(userDefinedClone != nullptr);
			uint32_t instanceid = (uint32_t) - 1;
			if (not instanciateAtomFunc(instanceid, (*userDefinedClone), /*void*/0, /*self*/lvid, rhs))
				return false;
			break;
		}
		default:
			return complain::multipleDefinitions(atom, "operator 'clone'");
	}
	Atom* clone = nullptr;
	switch (atom.findFuncAtom(clone, "^obj-clone")) {
		case 1: {
			assert(clone != nullptr);
			uint32_t instanceid = (uint32_t) - 1;
			if (instanciateAtomFunc(instanceid, (*clone), /*void*/0, /*self*/lvid, rhs)) {
				atom.classinfo.clone.atomid     = clone->atomid;
				atom.classinfo.clone.instanceid = instanceid;
				return true;
			}
			break;
		}
		case 0: {
			return complainMissingOperator(atom, "clone");
		}
		default:
			return complain::multipleDefinitions(atom, "operator 'obj-clone'");
	}
	return false;
}


bool SequenceBuilder::instanciateAtomClassDestructor(Atom& atom, uint32_t lvid) {
	assert(not signatureOnly);
	// if the ir code produced when transforming the AST is invalid,
	// a common scenario is that the code tries to destroy something (via unref)
	// which is not be a real class
	if (unlikely(not atom.isClass()))
		return (ice() << "trying to call the destructor of a non-class atom");
	if (unlikely(atom.flags(Atom::Flags::error)))
		return false;
	// first, try to find the user-defined dtor function (if any)
	Atom* userDefinedDtor = nullptr;
	switch (atom.findFuncAtom(userDefinedDtor, "^#user-dispose")) {
		case 0:
			break;
		case 1: {
			assert(userDefinedDtor != nullptr);
			uint32_t instanceid = static_cast<uint32_t>(-1);
			bool instok = instanciateAtomFunc(instanceid, (*userDefinedDtor), /*void*/0, /*self*/lvid);
			if (unlikely(not instok))
				return false;
			break;
		}
		default:
			return complain::multipleDefinitions(atom, "operator 'dispose'");
	}
	Atom* dtor = nullptr;
	switch (atom.findFuncAtom(dtor, "^dispose")) {
		case 1: {
			assert(dtor != nullptr);
			uint32_t instanceid = static_cast<uint32_t>(-1);
			if (instanciateAtomFunc(instanceid, (*dtor), /*void*/0, /*self*/lvid)) {
				atom.classinfo.dtor.atomid     = dtor->atomid;
				atom.classinfo.dtor.instanceid = instanceid;
				return true;
			}
			break;
		}
		case 0: {
			return complainMissingOperator(atom, "dispose");
		}
		default:
			return complain::multipleDefinitions(atom, "operator 'dispose'");
	}
	return false;
}


Atom* SequenceBuilder::instanciateAtomClass(Atom& atom) {
	assert(atom.isClass());
	// mark the atom being instanciated as 'instanciated'. For classes with gen. type parameters
	// a new atom will be created and only this one will be marked
	if (not atom.isContextual())
		atom.classinfo.isInstanciated = true;
	overloadMatch.clear(); // reset
	for (auto& param : pushedparams.gentypes.indexed)
		overloadMatch.input.tmplparams.indexed.emplace_back(frame->atomid, param.lvid);
	if (unlikely(not pushedparams.gentypes.named.empty()))
		error("named generic type parameters not implemented yet");
	TypeCheck::Match match = overloadMatch.validate(atom);
	if (unlikely(TypeCheck::Match::none == match)) {
		if (Config::Traces::sourceOpcodeSequence)
			debugPrintSourceOpcodeSequence(cdeftable, atom, "[FAIL-IR] ");
		// fail - try again to produce error message, hint, and any suggestion
		complainInvalidParametersAfterSignatureMatching(atom, overloadMatch);
		return nullptr;
	}
	// moving all input parameters
	decltype(FuncOverloadMatch::result.params) params;
	decltype(FuncOverloadMatch::result.params) tmplparams;
	params.swap(overloadMatch.result.params);
	tmplparams.swap(overloadMatch.result.tmplparams);
	Logs::Message::Ptr newReport;
	Pass::Instanciate::InstanciateData info{newReport, atom, cdeftable, build, params, tmplparams};
	info.parentAtom = &(frame->atom);
	info.shouldMergeLayer = true;
	info.parent = this;
	bool instanciated = Pass::Instanciate::instanciateAtom(info);
	report.subgroup().appendEntry(newReport);
	// !! The target atom may have changed here
	// (for any non contextual atoms, generic classes, anonymous classes...)
	auto& resAtom = info.atom.get();
	if (not instanciated) { // failed
		atom.flags += Atom::Flags::error;
		resAtom.flags += Atom::Flags::error;
		return nullptr;
	}
	return &resAtom;
}


bool SequenceBuilder::instanciateAtomFunc(uint32_t& instanceid, Atom& funcAtom, uint32_t retlvid, uint32_t p1,
		uint32_t p2) {
	assert(funcAtom.isFunction() or funcAtom.isTypeAlias());
	assert(frame != nullptr);
	auto& frameAtom = frame->atom;
	if (unlikely((p1 != 0 and not frame->verify(p1)) or (p2 != 0 and not frame->verify(p2)))) {
		if (retlvid != 0)
			frame->invalidate(retlvid);
		return false;
	}
	overloadMatch.clear(); // reset
	if (p1 != 0) { // first parameter
		overloadMatch.input.params.indexed.emplace_back(frameAtom.atomid, p1);
		if (p2 != 0) // second parameter
			overloadMatch.input.params.indexed.emplace_back(frameAtom.atomid, p2);
	}
	if (retlvid != 0) // return value
		overloadMatch.input.rettype.push_back(CLID{frameAtom.atomid, retlvid});
	TypeCheck::Match match = overloadMatch.validate(funcAtom);
	if (unlikely(TypeCheck::Match::none == match)) {
		// fail - try again to produce error message, hint, and any suggestion
		if (retlvid != 0)
			frame->invalidate(retlvid);
		return complainCannotCall(funcAtom, overloadMatch);
	}
	decltype(FuncOverloadMatch::result.params) params;
	decltype(FuncOverloadMatch::result.params) tmplparams;
	params.swap(overloadMatch.result.params);
	tmplparams.swap(overloadMatch.result.tmplparams);
	// instanciate the called func
	Logs::Message::Ptr subreport;
	InstanciateData info{subreport, funcAtom, cdeftable, build, params, tmplparams};
	bool instok = doInstanciateAtomFunc(subreport, info, retlvid);
	instanceid = info.instanceid;
	if (unlikely(not instok and retlvid != 0))
		frame->invalidate(retlvid);
	return instok;
}


bool SequenceBuilder::doInstanciateAtomFunc(Logs::Message::Ptr& subreport, InstanciateData& info,
		uint32_t retlvid) {
	// even within a typeof, any new instanciation must see their code generated
	// (and its errors generated)
	info.canGenerateCode = true; // canGenerateCode();
	info.parent = this;
	bool instanciated = instanciateAtom(info);
	report.appendEntry(subreport);
	if (unlikely(not instanciated))
		return (success = false);
	if (unlikely(info.instanceid == static_cast<uint32_t>(-1)))
		return complain::classdef(info.returnType, "return: invalid instance id");
	// import the return type of the instanciated sequence
	if (retlvid != 0) {
		auto& spare = cdeftable.substitute(retlvid);
		if (not info.returnType.isVoid()) {
			spare.kind  = info.returnType.kind;
			spare.atom  = info.returnType.atom;
			spare.instance = true; // force some values just in case
			if (unlikely(not spare.isBuiltinOrVoid() and spare.atom == nullptr))
				return (ice() << "return: invalid atom for return type");
			frame->lvids(retlvid).synthetic = false;
			// release automatically the returned value, acquired by the function
			if (canBeAcquired(info.returnType))
				frame->lvids(retlvid).autorelease = true;
		}
		else
			spare.mutateToVoid();
		frame->lvids(retlvid).origin.returnedValue = true;
	}
	return true;
}


bool SequenceBuilder::getReturnTypeForRecursiveFunc(const Atom& atom, Classdef& rettype) const {
	// looking for the parent sequence builder currently generating ir for this atom
	// since the func is not fully instanciated yet, the real types are kept by cdeftable
	const SequenceBuilder* parentBuilder = nullptr;
	auto atomid = atom.atomid;
	for (auto* sb = this; sb != nullptr; ) {
		auto* builderIT = sb;
		sb = sb->parent;
		for (auto* f = builderIT->frame; f != nullptr; f = f->previous) {
			if (f->atom.atomid == atomid) {
				sb = nullptr;
				parentBuilder = builderIT;
				break;
			}
		}
	}
	if (unlikely(!parentBuilder))
		return (ice() << "failed to find parent sequence builder");
	if (atom.hasGenericParameters())
		return error("recursive functions with generic type parameters is not supported yet");
	// !! NOTE !!
	// the func is not fully instanciated, so the real return type is not set yet
	// (only at the end of the instanciation).
	// However, the user-type must be already resolved in {atomid:1}, where :1 is the user return type
	// the same here, it is quite probable that a substite has been added without the complete type
	// using the 'rawclassdef' to resolve any hard link first
	// checking the return type
	if (not atom.returnType.clid.isVoid()) {
		auto& raw  = parentBuilder->cdeftable.rawclassdef(CLID{atomid, 1});
		auto& cdef = parentBuilder->cdeftable.classdef(raw.clid);
		if (not cdef.isBuiltinOrVoid()) {
			Atom* retatom = parentBuilder->cdeftable.findClassdefAtom(cdef);
			if (unlikely(!retatom))
				return false;
			rettype.mutateToAtom(retatom);
			rettype.qualifiers = cdef.qualifiers;
		}
		else
			rettype.mutateToVoid();
	}
	else
		rettype.mutateToVoid();
	// checking each parameters
	for (uint32_t p = 0; p != atom.parameters.size(); ++p) {
		auto& clid = atom.parameters.vardef(p).clid;
		// cf notes above
		auto& raw  = parentBuilder->cdeftable.rawclassdef(clid);
		auto& cdef = parentBuilder->cdeftable.classdef(raw.clid);
		if (not cdef.isBuiltinOrVoid()) {
			if (unlikely(nullptr == parentBuilder->cdeftable.findClassdefAtom(cdef)))
				return false;
		}
	}
	return true;
}


bool instanciateAtomParameterTypes(InstanciateData& info) {
	// Despite the location of this code, no real code instanciation
	// of any sort will be done (the code is the same, that's why).
	// This pass only intends to resolve user-given types for parameters
	// example:
	//    func foo(p1, p2: UserType)
	// The second parameter will be of interest in this case
	// The sequence builder will stop as soon as the opcode 'bodystart'
	// is encountered
	Signature signature;
	// the current atom, probably different from `previousAtom`
	auto& atom = info.atom.get();
	// the original ir sequence generated from the AST
	auto& inputIR = *(atom.opcodes.sequence);
	// no output
	ir::Sequence* outIR = nullptr;
	// new layer for the cdeftable
	ClassdefTableView newview{info.cdeftable, atom.atomid, signature.parameters.size()};
	// log
	Logs::Report report{*info.report};
	if (unlikely(!atom.parent))
		return (ice() << "invalid atom, no parent");
	if (!atom.opcodes.sequence) {
		// type alias for template classes can be empty (generated by the compiler)
		if (atom.isTypeAlias())
			return true;
		return (ice() << "invalid atom: no ir code");
	}
	// instanciate the sequence attached to the atom
	auto builder = std::make_unique<SequenceBuilder>
				   (report.subgroup(), newview, info.build, outIR, inputIR, info.parent);
	//if (info.parentAtom)
	builder->layerDepthLimit = 2; // allow the first blueprint to be instanciated
	builder->signatureOnly = true;
	builder->codeGenerationLock = 666; // arbitrary value != 0 to prevent from code generation
	// Atom ID mapping, irelevant here
	builder->mappingBlueprintAtomID[0] = atom.atomid;
	builder->mappingBlueprintAtomID[1] = atom.atomid;
	bool instanciated = builder->readAndInstanciate(atom.opcodes.offset);
	assert(builder->codeGenerationLock == 666);
	if (not instanciated)
		return false;
	auto mergeType = [&](const CLID & clid) {
		auto& cdef = newview.classdef(clid);
		auto& rawcdef = newview.originalTable().rawclassdef(clid);
		rawcdef.qualifiers = cdef.qualifiers;
		if (not cdef.isBuiltinOrVoid()) {
			auto* useratom = newview.findClassdefAtom(cdef);
			rawcdef.mutateToAtom(useratom);
		}
		else {
			rawcdef.qualifiers.ref = false; // no ref for builtin types
			rawcdef.mutateToBuiltinOrVoid(cdef.kind);
		}
	};
	// import parameter types
	atom.parameters.each([&](uint32_t, const AnyString&, const Vardef & vardef) {
		mergeType(vardef.clid);
	});
	mergeType(CLID{atom.atomid, 1}); // return type
	return true;
}


bool instanciateAtom(InstanciateData& info) {
	// prepare the matching signature
	Signature signature;
	prepareSignature(signature, info);
	assert(info.params.size() == signature.parameters.size());
	// the atom being instanciated
	auto& atom = info.atom.get();
	// Another atom, if the target atom had changed
	Atom* remapAtom = nullptr;
	auto valid = atom.instances.isValid(signature, info.instanceid, info.returnType, remapAtom);
	switch (valid) {
		case Tribool::Value::yes: {
			if (unlikely(atom.flags(Atom::Flags::instanciating))) { // recursive func detected
				if (unlikely(not instanciateRecursiveAtom(info)))
					return false;
			}
			// instance already present
			if (unlikely(remapAtom != nullptr)) // the target atom may have changed (template class)
				info.atom = std::ref(*remapAtom);
			return true;
		}
		case Tribool::Value::indeterminate: {
			// the atom must be instanciated
			return (nullptr != performAtomInstanciation(info, signature));
		}
		case Tribool::Value::no: {
			// failed to instanciate last time. error already reported
			break;
		}
	}
	return false;
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
