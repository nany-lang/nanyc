#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	bool ProgramBuilder::instanciateIntrinsicFieldset(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_void;
		// params
		if (unlikely(not checkForIntrinsicParamCount("__nanyc_fieldset", 2)))
			return false;

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


	bool ProgramBuilder::instanciateIntrinsicRef(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_void;
		// params
		if (unlikely(not checkForIntrinsicParamCount("ref", 1)))
			return false;

		if (canGenerateCode())
			tryToAcquireObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	bool ProgramBuilder::instanciateIntrinsicUnref(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_void;
		// params
		if (unlikely(not checkForIntrinsicParamCount("unref", 1)))
			return false;

		if (canGenerateCode())
			tryUnrefObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	bool ProgramBuilder::instanciateIntrinsicAddressof(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_u64;
		// params
		if (unlikely(not checkForIntrinsicParamCount("addressof", 1)))
			return false;

		if (canGenerateCode())
		{
			uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
			if (canBeAcquired(objlvid))
				out.emitStore(lvid, objlvid);
			else
				out.emitStoreConstant(lvid, (uint64_t) 0);
		}
		return true;
	}


	bool ProgramBuilder::instanciateIntrinsicSizeof(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_u64;
		// params
		if (unlikely(not checkForIntrinsicParamCount("sizeof", 1)))
			return false;

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
				out.emitStoreConstant(lvid, size);
			}
		}
		return true;
	}


	bool ProgramBuilder::instanciateIntrinsicMemalloc(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_u64;
		// paramates
		if (unlikely(not checkForIntrinsicParamCount("memory.allocate", 1)))
			return false;

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& frame = atomStack.back();
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});
		if (not cdef.isBuiltingUnsigned())
			return complainIntrinsicParameter("memory.allocate", 0, cdef, "'__u64'");

		if (canGenerateCode())
			out.emitMemalloc(lvid, objlvid);
		return true;
	}


	bool ProgramBuilder::instanciateIntrinsicMemFree(uint32_t lvid)
	{
		// no return value
		cdeftable.substitute(lvid).kind = nyt_void;
		// paramates
		if (unlikely(not checkForIntrinsicParamCount("memory.dispose", 2)))
			return false;

		auto& frame = atomStack.back();

		uint32_t objlvid = lastPushedIndexedParameters[0].lvid;
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame.atomid, objlvid});
		if (cdef.kind != nyt_u64)
			return complainIntrinsicParameter("memory.dispose", 0, cdef, "'__u64'");

		uint32_t size = lastPushedIndexedParameters[1].lvid;
		auto& cdefsize = cdeftable.classdefFollowClassMember(CLID{frame.atomid, size});
		if (not cdefsize.isBuiltingUnsigned())
			return complainIntrinsicParameter("memory.dispose", 1, cdef, "'__u64'");

		if (canGenerateCode())
			out.emitMemFree(objlvid, size);
		return true;
	}







	bool ProgramBuilder::instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid)
	{
		assert(not name.empty());
		switch (name[0])
		{
			case '_':
			{
				if (name == "__nanyc_fieldset")
					return instanciateIntrinsicFieldset(lvid);
				break;
			}
			case 'a':
			{
				if (name == "addressof")
					return instanciateIntrinsicAddressof(lvid);
				break;
			}
			case 'm':
			{
				if (name == "memory.allocate")
					return instanciateIntrinsicMemalloc(lvid);
				if (name == "memory.dispose")
					return instanciateIntrinsicMemFree(lvid);
				break;
			}
			case 'r':
			{
				if (name == "ref")
					return instanciateIntrinsicRef(lvid);
				break;
			}
			case 's':
			{
				if (name == "sizeof")
					return instanciateIntrinsicSizeof(lvid);
				break;
			}
			case 'u':
			{
				if (name == "unref")
					return instanciateIntrinsicUnref(lvid);
				break;
			}
		}
		return (error() << "unknown intrinsic '" << name << '\'');
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
