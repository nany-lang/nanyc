#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	inline bool ProgramBuilder::instanciateIntrinsicFieldset(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_void;

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


	inline bool ProgramBuilder::instanciateIntrinsicRef(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_void;
		if (canGenerateCode())
			tryToAcquireObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	inline bool ProgramBuilder::instanciateIntrinsicUnref(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_void;
		if (canGenerateCode())
			tryUnrefObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	inline bool ProgramBuilder::instanciateIntrinsicAddressof(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_u64;

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


	inline bool ProgramBuilder::instanciateIntrinsicSizeof(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_u64;

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


	inline bool ProgramBuilder::instanciateIntrinsicMemalloc(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_u64;

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& frame = atomStack.back();
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});
		if (not cdef.isBuiltingUnsigned())
			return complainIntrinsicParameter("memory.allocate", 0, cdef, "'__u64'");

		if (canGenerateCode())
			out.emitMemalloc(lvid, objlvid);
		return true;
	}


	inline bool ProgramBuilder::instanciateIntrinsicMemFree(uint32_t lvid)
	{
		cdeftable.substitute(lvid).kind = nyt_void;

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
	};

	bool ProgramBuilder::instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid)
	{
		assert(not name.empty());
		auto it = builtinDispatch.find(name);
		if (likely(it != builtinDispatch.end()))
		{
			// check for parameter count
			if (unlikely(not checkForIntrinsicParamCount(name, it->second.second)))
				return false;

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
