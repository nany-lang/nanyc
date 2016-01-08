#include "instanciate.h"
#include "details/type/type-check.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	bool  ProgramBuilder::instanciateIntrinsicFieldset(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();

		auto& frame = atomStack.back();
		uint32_t lvidsid = lastPushedIndexedParameters[1].lvid;
		uint32_t sid = frame.lvids[lvidsid].text_sid;
		if (unlikely(sid == (uint32_t) -1))
			return (ICE() << "invalid string-id for field name");

		AnyString varname = out.stringrefs[sid];
		if (unlikely(varname.empty()))
			return (ICE() << "invalid empty field name");

		Atom* parent = frame.atom.isClass() ? &frame.atom : frame.atom.parent;
		if (unlikely(!parent))
			return (ICE() << "invalid parent atom for __nanyc_fieldset");

		Atom* varatom = nullptr;
		parent->eachChild(varname, [&](Atom& child) -> bool {
			if (unlikely(varatom))
				return (ICE() << "duplicate variable member '" << varname << "'");
			varatom = &child;
			return true;
		});
		if (unlikely(!varatom))
			return (ICE() << "invalid variable member atom for __nanyc_fieldset");

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& cdefvar = cdeftable.classdefFollowClassMember(varatom->returnType.clid);
		auto& cdef    = cdeftable.classdef(CLID{frame.atomid, objlvid});

		// flag for implicitly converting objects (bool, i32...) into builtin (__bool, __i32...)
		bool implicitBuiltin = cdefvar.isBuiltin() and (not cdef.isBuiltinOrVoid());
		if (implicitBuiltin)
		{
			// checking if an implicit can be performed (if rhs is a 'builtin' type)
			auto* atomrhs = (cdeftable.findClassdefAtom(cdef));
			implicitBuiltin = (cdeftable.atoms().core.object[cdefvar.kind] == atomrhs);
		}

		if (not implicitBuiltin)
		{
			auto similarity = TypeCheck::isSimilarTo(cdeftable, nullptr, cdef, cdefvar);
			if (unlikely(TypeCheck::Match::none == similarity))
				return complainInvalidType(cdef, cdefvar);
		}

		if (canGenerateCode())
		{
			if (not implicitBuiltin)
			{
				tryToAcquireObject(objlvid);
				out.emitFieldset(objlvid, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
			}
			else
			{
				uint32_t lvidvalue = createLocalVariables();
				out.emitFieldget(lvidvalue, objlvid, 0);
				out.emitFieldset(lvidvalue, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
			}
		}
		return true;
	}


	bool  ProgramBuilder::instanciateIntrinsicRef(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();
		if (canGenerateCode())
			tryToAcquireObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	bool  ProgramBuilder::instanciateIntrinsicUnref(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();
		if (canGenerateCode())
			tryUnrefObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	bool  ProgramBuilder::instanciateIntrinsicAddressof(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToBuiltin(nyt_u64);

		if (canGenerateCode())
		{
			uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
			if (canBeAcquired(objlvid))
				out.emitStore(lvid, objlvid);
			else
				out.emitStore_u64(lvid, 0);
		}
		return true;
	}


	bool  ProgramBuilder::instanciateIntrinsicSizeof(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToBuiltin(nyt_u64);

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& frame = atomStack.back();
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});

		bool isBuiltinOrVoid = cdef.isBuiltinOrVoid();
		Atom* atom = nullptr;
		if (not isBuiltinOrVoid)
		{
			atom = cdeftable.findClassdefAtom(cdef);
			if (unlikely(nullptr == atom))
				return (ICE() << "invalid atom for sizeof operator");
		}

		if (canGenerateCode())
		{
			uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
			auto& frame = atomStack.back();
			auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});

			if (not isBuiltinOrVoid)
			{
				assert(atom != nullptr);
				out.emitSizeof(lvid, atom->atomid);
			}
			else
			{
				uint64_t size = nany_type_sizeof(cdef.kind);
				out.emitStore_u64(lvid, size);
			}
		}
		return true;
	}


	bool  ProgramBuilder::instanciateIntrinsicMemalloc(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToBuiltin(nyt_u64);

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& frame = atomStack.back();
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});
		if (not cdef.isBuiltingUnsigned())
			return complainIntrinsicParameter("memory.allocate", 0, cdef, "'__u64'");

		if (canGenerateCode())
			out.emitMemalloc(lvid, objlvid);
		return true;
	}


	bool  ProgramBuilder::instanciateIntrinsicMemFree(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();

		auto& frame = atomStack.back();

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});
		if (cdef.kind != nyt_u64)
			return complainIntrinsicParameter("memory.dispose", 0, cdef, "'__u64'");

		uint32_t size = lastPushedIndexedParameters[1].lvid;
		auto& cdefsize = cdeftable.classdefFollowClassMember(CLID{frame.atomid, size});
		if (unlikely(not cdefsize.isBuiltingUnsigned()))
			return complainIntrinsicParameter("memory.dispose", 1, cdef, "'__u64'");

		if (canGenerateCode())
			out.emitMemFree(objlvid, size);
		return true;
	}


	template<void (IR::Program::* M)(uint32_t, uint32_t, uint32_t)>
	inline bool ProgramBuilder::instanciateIntrinsicLogic(uint32_t lvid, const char* const name)
	{
		auto& frame = atomStack.back();
		uint32_t lhs  = lastPushedIndexedParameters[0].lvid;
		auto& cdeflhs   = cdeftable.classdefFollowClassMember(CLID{frame.atomid, lhs});
		uint32_t rhs  = lastPushedIndexedParameters[1].lvid;
		auto& cdefrhs = cdeftable.classdefFollowClassMember(CLID{frame.atomid, rhs});

		nytype_t builtinlhs = cdeflhs.kind;
		if (builtinlhs == nyt_any)
		{
			auto* atom = cdeftable.findClassdefAtom(cdeflhs);
			if (atom != nullptr and atom->builtinMapping != nyt_void)
			{
				builtinlhs = atom->builtinMapping;
				uint32_t newlvid = createLocalVariables();
				out.emitFieldget(newlvid, lhs, 0);
				lhs = newlvid;
			}
		}

		nytype_t builtinrhs = cdefrhs.kind;
		if (builtinrhs == nyt_any)
		{
			auto* atom = cdeftable.findClassdefAtom(cdefrhs);
			if (atom != nullptr and atom->builtinMapping != nyt_void)
			{
				builtinrhs = atom->builtinMapping;
				uint32_t newlvid = createLocalVariables();
				out.emitFieldget(newlvid, rhs, 0);
				rhs = newlvid;
			}
		}

		if (unlikely(builtinlhs != builtinrhs))
		{
			return complainIntrinsicParameter(name, 0, cdeflhs);
			return complainIntrinsicParameter(name, 1, cdefrhs);
		}

		if (unlikely(builtinlhs == nyt_void or builtinlhs == nyt_any or builtinlhs == nyt_pointer))
			return complainIntrinsicParameter(name, 0, cdeflhs);
		if (unlikely(builtinrhs == nyt_void or builtinrhs == nyt_any or builtinrhs == nyt_pointer))
			return complainIntrinsicParameter(name, 1, cdefrhs);


		cdeftable.substitute(lvid).mutateToBuiltin(builtinlhs);
		if (canGenerateCode())
			(out.*M)(lvid, lhs, rhs);
		return true;
	}

	bool  ProgramBuilder::instanciateIntrinsicAND(uint32_t lvid)
	{
		return instanciateIntrinsicLogic<&IR::Program::emitAND>(lvid, "and");
	}

	bool  ProgramBuilder::instanciateIntrinsicOR(uint32_t lvid)
	{
		return instanciateIntrinsicLogic<&IR::Program::emitOR>(lvid, "or");
	}

	bool  ProgramBuilder::instanciateIntrinsicXOR(uint32_t lvid)
	{
		return instanciateIntrinsicLogic<&IR::Program::emitXOR>(lvid, "xor");
	}

	bool  ProgramBuilder::instanciateIntrinsicMOD(uint32_t lvid)
	{
		return instanciateIntrinsicLogic<&IR::Program::emitMOD>(lvid, "mod");
	}









	typedef bool (ProgramBuilder::* BuiltinIntrinsic)(uint32_t);

	static const std::unordered_map<AnyString, std::pair<BuiltinIntrinsic, uint32_t>> builtinDispatch =
	{
		{"^fieldset",       { &ProgramBuilder::instanciateIntrinsicFieldset,  2 }},
		{"addressof",       { &ProgramBuilder::instanciateIntrinsicAddressof, 1 }},
		{"memory.allocate", { &ProgramBuilder::instanciateIntrinsicMemalloc,  1 }},
		{"memory.dispose",  { &ProgramBuilder::instanciateIntrinsicMemFree,   2 }},
		{"ref",             { &ProgramBuilder::instanciateIntrinsicRef,       1 }},
		{"unref",           { &ProgramBuilder::instanciateIntrinsicUnref,     1 }},
		{"sizeof",          { &ProgramBuilder::instanciateIntrinsicSizeof,    1 }},
		//
		{"and",             { &ProgramBuilder::instanciateIntrinsicAND,       2 }},
		{"or",              { &ProgramBuilder::instanciateIntrinsicOR,        2 }},
		{"xor",             { &ProgramBuilder::instanciateIntrinsicXOR,       2 }},
		{"mod",             { &ProgramBuilder::instanciateIntrinsicMOD,       2 }},
	};

	bool ProgramBuilder::instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid)
	{
		assert(not name.empty());
		if (unlikely(not lastPushedNamedParameters.empty()))
			return (error() << "intrinsics do not accept named parameters");

		auto it = builtinDispatch.find(name);
		if (unlikely(it == builtinDispatch.end()))
			return (error() << "unknown intrinsic '" << name << '\'');

		// check for parameters
		{
			auto& frame = atomStack.back();
			uint32_t count = it->second.second;
			if (unlikely(not checkForIntrinsicParamCount(name, count)))
				return false;

			// checking if one parameter was already flag as 'error'
			for (uint32_t i = 0u; i != count; ++i)
			{
				if (unlikely(not frame.verify(lastPushedIndexedParameters[i].lvid)))
					return false;
			}
		}

		// specific code for the intrinsic
		bool success = (this->*(it->second.first))(lvid);

		// annotate any error
		if (unlikely(not success))
			atomStack.back().lvids[lvid].errorReported = true;
		return success;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
