#include "instanciate.h"

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


		if (canGenerateCode())
		{
			uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
			tryToAcquireObject(objlvid);
			out.emitFieldset(objlvid, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
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
		auto& cdeflhs = cdeftable.classdefFollowClassMember(CLID{frame.atomid, lhs});
		uint32_t rhs  = lastPushedIndexedParameters[1].lvid;
		auto& cdefrhs = cdeftable.classdefFollowClassMember(CLID{frame.atomid, rhs});

		if (unlikely(not cdeflhs.isBuiltin()))
			return complainIntrinsicParameter(name, 0, cdeflhs, "'__u64'");
		if (unlikely(not cdefrhs.isBuiltin()))
			return complainIntrinsicParameter(name, 1, cdefrhs, "'__u64'");

		if (unlikely(cdeflhs.kind != cdefrhs.kind))
		{
			return complainIntrinsicParameter(name, 0, cdeflhs, "'__u64'");
			return complainIntrinsicParameter(name, 1, cdefrhs, "'__u64'");
		}

		cdeftable.substitute(lvid).mutateToBuiltin(cdeflhs.kind);
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
		{"__nanyc_fieldset", { &ProgramBuilder::instanciateIntrinsicFieldset,  2 }},
		{"addressof",        { &ProgramBuilder::instanciateIntrinsicAddressof, 1 }},
		{"memory.allocate",  { &ProgramBuilder::instanciateIntrinsicMemalloc,  1 }},
		{"memory.dispose",   { &ProgramBuilder::instanciateIntrinsicMemFree,   2 }},
		{"ref",              { &ProgramBuilder::instanciateIntrinsicRef,       1 }},
		{"unref",            { &ProgramBuilder::instanciateIntrinsicUnref,     1 }},
		{"sizeof",           { &ProgramBuilder::instanciateIntrinsicSizeof,    1 }},
		//
		{"and",              { &ProgramBuilder::instanciateIntrinsicAND,       2 }},
		{"or",               { &ProgramBuilder::instanciateIntrinsicOR,        2 }},
		{"xor",              { &ProgramBuilder::instanciateIntrinsicXOR,       2 }},
		{"mod",              { &ProgramBuilder::instanciateIntrinsicMOD,       2 }},
	};

	bool ProgramBuilder::instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid)
	{
		assert(not name.empty());
		auto it = builtinDispatch.find(name);
		if (likely(it != builtinDispatch.end()))
		{
			// check for parameters
			{
				auto& frame = atomStack.back();
				uint32_t count = it->second.second;
				if (unlikely(not checkForIntrinsicParamCount(name, count)))
					return false;

				// checking if one parameter was already flag as 'error'
				for (uint32_t i = 0; i != count; ++i)
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
		return (error() << "unknown intrinsic '" << name << '\'');
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
