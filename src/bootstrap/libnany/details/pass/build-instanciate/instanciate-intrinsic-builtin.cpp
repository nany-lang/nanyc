#include "instanciate.h"
#include "type-check.h"
#ifdef YUNI_OS_UNIX
#include <unistd.h>
#endif

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	namespace // nonymous
	{

		static bool intrinsicMemcheckerHold(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdefptr = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (not cdefptr.isRawPointer())
				return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefptr, "'__pointer'");

			uint32_t sizelvid = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, sizelvid});
			if (not cdefsize.isBuiltinU64())
				return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefsize, "'__u64'");

			if (seq.canGenerateCode())
				seq.out.emitMemcheckhold(ptrlvid, sizelvid);
			return true;
		}


		static bool intrinsicOSIsUnix(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#ifdef YUNI_OS_UNIX
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}

		static bool intrinsicOSIsPosix(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#if defined(YUNI_OS_UNIX) && defined(_POSIX_VERSION)
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}


		static bool intrinsicOSIsLinux(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#ifdef YUNI_OS_LINUX
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}

		static bool intrinsicOSIsAIX(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#ifdef YUNI_OS_AIX
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}

		static bool intrinsicOSIsWindows(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#ifdef YUNI_OS_WINDOWS
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}

		static bool intrinsicOSIsCygwin(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#if defined(__CYGWIN32__) || defined(__CYGWIN__)
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}

		static bool intrinsicOSIsMacOS(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#ifdef YUNI_OS_MACOS
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}

		static bool intrinsicOSIsBSD(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_bool);
			#if defined(YUNI_OS_MACOS) || defined(YUNI_OS_OPENBSD) || defined(YUNI_OS_FREEBSD) \
				|| defined(YUNI_OS_NETBSD) || defined(YUNI_OS_DRAGONFLY)
			seq.out.emitStore_u64(lvid, 1);
			#else
			seq.out.emitStore_u64(lvid, 0);
			#endif
			return true;
		}



		static bool intrinsicFieldset(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t lvidsid = seq.pushedparams.func.indexed[1].lvid;
			uint32_t sid = seq.frame->lvids[lvidsid].text_sid;
			if (unlikely(sid == (uint32_t) -1))
				return (ice() << "invalid string-id for field name (got lvid " << lvidsid << ')');

			AnyString varname = seq.out.stringrefs[sid];
			if (unlikely(varname.empty()))
				return (ice() << "invalid empty field name");

			Atom* parent = seq.frame->atom.isClass() ? &seq.frame->atom : seq.frame->atom.parent;
			if (unlikely(!parent))
				return (ice() << "invalid parent atom for __nanyc_fieldset");

			Atom* varatom = nullptr;
			parent->eachChild(varname, [&](Atom& child) -> bool {
				if (unlikely(varatom))
					return (ice() << "duplicate variable member '" << varname << "'");
				varatom = &child;
				return true;
			});
			if (unlikely(!varatom))
				return (ice() << "invalid variable member atom for __nanyc_fieldset");

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdefvar = seq.cdeftable.classdefFollowClassMember(varatom->returnType.clid);
			auto& cdef    = seq.cdeftable.classdef(CLID{seq.frame->atomid, objlvid});

			// flag for implicitly converting objects (bool, i32...) into builtin (__bool, __i32...)
			bool implicitBuiltin = cdefvar.isBuiltin() and (not cdef.isBuiltinOrVoid());
			if (implicitBuiltin)
			{
				// checking if an implicit can be performed (if rhs is a 'builtin' type)
				auto* atomrhs = (seq.cdeftable.findClassdefAtom(cdef));
				implicitBuiltin = (seq.cdeftable.atoms().core.object[cdefvar.kind] == atomrhs);
			}

			if (not implicitBuiltin)
			{
				auto similarity = TypeCheck::isSimilarTo(seq, cdef, cdefvar);
				if (unlikely(TypeCheck::Match::none == similarity))
					return seq.complainInvalidType("fieldset: ", cdef, cdefvar);
			}

			if (seq.canGenerateCode())
			{
				if (not implicitBuiltin)
				{
					seq.tryToAcquireObject(objlvid);
					seq.out.emitFieldset(objlvid, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
				}
				else
				{
					uint32_t lvidvalue = seq.createLocalVariables();
					seq.out.emitFieldget(lvidvalue, objlvid, 0);
					seq.out.emitFieldset(lvidvalue, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);
				}
			}
			return true;
		}


		static bool intrinsicRef(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();
			if (seq.canGenerateCode())
				seq.tryToAcquireObject(seq.pushedparams.func.indexed[0].lvid);
			return true;
		}


		static bool intrinsicUnref(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();
			if (seq.canGenerateCode())
				seq.tryUnrefObject(seq.pushedparams.func.indexed[0].lvid);
			return true;
		}


		static bool intrinsicPointer(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_ptr);

			if (seq.canGenerateCode())
			{
				uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
				if (seq.canBeAcquired(objlvid))
				{
					// retrieving the pointer address of the object in input
					// the objlvid representing the object is already the address
					seq.out.emitStore(lvid, objlvid);
				}
				else
				{
					// can not be acquired, not an object - NULL
					seq.out.emitStore_u64(lvid, 0);
				}
			}
			return true;
		}


		static bool intrinsicSizeof(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_u64);

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});

			bool isBuiltinOrVoid = cdef.isBuiltinOrVoid();
			Atom* atom = nullptr;
			if (not isBuiltinOrVoid)
			{
				atom = seq.cdeftable.findClassdefAtom(cdef);
				if (unlikely(nullptr == atom))
					return (ice() << "invalid atom for sizeof operator");
			}

			if (seq.canGenerateCode())
			{
				uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
				auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});

				if (not isBuiltinOrVoid)
				{
					assert(atom != nullptr);
					seq.out.emitSizeof(lvid, atom->atomid);
				}
				else
				{
					uint64_t size = nany_type_sizeof(cdef.kind);
					seq.out.emitStore_u64(lvid, size);
				}
			}
			return true;
		}


		static bool intrinsicMemalloc(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_ptr);

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
			if (not cdef.isBuiltinU64())
				return seq.complainIntrinsicParameter("memory.allocate", 0, cdef, "'__u64'");

			if (seq.canGenerateCode())
				seq.out.emitMemalloc(lvid, objlvid);
			return true;
		}


		static bool intrinsicMemrealloc(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_ptr);

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdefptr = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (not cdefptr.isRawPointer())
				return seq.complainIntrinsicParameter("memory.realloc", 0, cdefptr, "'__pointer'");

			uint32_t oldsizelvid = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefOldsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, oldsizelvid});
			if (not cdefOldsize.isBuiltinU64())
				return seq.complainIntrinsicParameter("memory.realloc", 0, cdefOldsize, "'__u64'");

			uint32_t newsizelvid = seq.pushedparams.func.indexed[2].lvid;
			auto& cdefNewsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, newsizelvid});
			if (not cdefNewsize.isBuiltinU64())
				return seq.complainIntrinsicParameter("memory.realloc", 0, cdefNewsize, "'__u64'");

			if (seq.canGenerateCode())
			{
				seq.out.emitStore(lvid, ptrlvid);
				seq.out.emitMemrealloc(lvid, oldsizelvid, newsizelvid);
			}
			return true;
		}


		static bool intrinsicMemFree(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
			if (cdef.kind != nyt_ptr)
				return seq.complainIntrinsicParameter("memory.dispose", 0, cdef, "'__u64'");

			uint32_t size = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
			if (unlikely(not cdefsize.isBuiltingUnsigned()))
				return seq.complainIntrinsicParameter("memory.dispose", 1, cdef, "'__u64'");

			if (seq.canGenerateCode())
				seq.out.emitMemFree(objlvid, size);
			return true;
		}


		static bool intrinsicMemfill(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
			if (cdef.kind != nyt_ptr)
				return seq.complainIntrinsicParameter("memory.memset", 0, cdef, "'__pointer'");

			uint32_t size = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
			if (unlikely(not cdefsize.isBuiltingUnsigned()))
				return seq.complainIntrinsicParameter("memory.memset", 1, cdef, "'__u64'");

			uint32_t patternlvid = seq.pushedparams.func.indexed[2].lvid;
			auto& cdefpattern = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, patternlvid});
			if (unlikely(not cdefpattern.isBuiltingUnsigned()))
				return seq.complainIntrinsicParameter("memory.memset", 2, cdef, "'__u32'");

			if (seq.canGenerateCode())
				seq.out.emitMemFill(objlvid, size, patternlvid);
			return true;
		}

		static bool intrinsicMemCopy(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("memory.copy", 0, cdef, "'__pointer'");

			uint32_t src = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefsrc = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, src});
			if (unlikely(not cdefsrc.isRawPointer()))
				return seq.complainIntrinsicParameter("memory.copy", 1, cdef, "'__pointer'");

			uint32_t size = seq.pushedparams.func.indexed[2].lvid;
			auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
			if (unlikely(not cdefsize.isBuiltinU64()))
				return seq.complainIntrinsicParameter("memory.copy", 2, cdef, "'__u64'");

			if (seq.canGenerateCode())
				seq.out.emitMemCopy(objlvid, src, size);
			return true;
		}

		static bool intrinsicMemMove(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("memory.move", 0, cdef, "'__pointer'");

			uint32_t src = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefsrc = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, src});
			if (unlikely(not cdefsrc.isRawPointer()))
				return seq.complainIntrinsicParameter("memory.move", 1, cdef, "'__pointer'");

			uint32_t size = seq.pushedparams.func.indexed[2].lvid;
			auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
			if (unlikely(not cdefsize.isBuiltinU64()))
				return seq.complainIntrinsicParameter("memory.move", 2, cdef, "'__u64'");

			if (seq.canGenerateCode())
				seq.out.emitMemMove(objlvid, src, size);
			return true;
		}

		static bool intrinsicMemCmp(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_u64);

			uint32_t objlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, objlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("memory.cmp", 0, cdef, "'__pointer'");

			uint32_t src = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefsrc = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, src});
			if (unlikely(not cdefsrc.isRawPointer()))
				return seq.complainIntrinsicParameter("memory.cmp", 1, cdef, "'__pointer'");

			uint32_t size = seq.pushedparams.func.indexed[2].lvid;
			auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, size});
			if (unlikely(not cdefsize.isBuiltinU64()))
				return seq.complainIntrinsicParameter("memory.cmp", 2, cdef, "'__u64'");

			if (seq.canGenerateCode())
			{
				seq.out.emitStore(lvid, size);
				seq.out.emitMemCmp(objlvid, src, lvid);
			}
			return true;
		}


		static bool intrinsicMemGetU64(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_u64);

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("load.u64", 0, cdef, "'__pointer'");

			if (seq.canGenerateCode())
				seq.out.emitLoadU64(lvid, ptrlvid);
			return true;
		}

		static bool intrinsicMemGetU32(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_u32);

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("load.u32", 0, cdef, "'__pointer'");

			if (seq.canGenerateCode())
				seq.out.emitLoadU32(lvid, ptrlvid);
			return true;
		}

		static bool intrinsicMemGetU8(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_u8);

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("load.u8", 0, cdef, "'__pointer'");

			if (seq.canGenerateCode())
				seq.out.emitLoadU8(lvid, ptrlvid);
			return true;
		}

		static bool intrinsicMemGetPTR(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToBuiltin(nyt_ptr);

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("load.ptr", 0, cdef, "'__pointer'");

			if (seq.canGenerateCode())
			{
				if (sizeof(uint64_t) == sizeof(void*))
					seq.out.emitLoadU64(lvid, ptrlvid);
				else
					seq.out.emitLoadU32(lvid, ptrlvid);
			}
			return true;
		}

		static bool intrinsicMemSetU64(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("store.u64", 0, cdef, "'__pointer'");

			uint32_t value = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
			if (unlikely(not cdefvalue.isBuiltinU64()))
				return seq.complainIntrinsicParameter("store.u64", 1, cdefvalue, "'__u64'");

			if (seq.canGenerateCode())
				seq.out.emitStoreU64(value, ptrlvid);
			return true;
		}

		static bool intrinsicMemSetU32(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("store.u32", 0, cdef, "'__pointer'");

			uint32_t value = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
			if (unlikely(not cdefvalue.isBuiltinU32()))
				return seq.complainIntrinsicParameter("store.u32", 1, cdefvalue, "'__u32'");

			if (seq.canGenerateCode())
				seq.out.emitStoreU32(value, ptrlvid);
			return true;
		}

		static bool intrinsicMemSetU8(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("store.u8", 0, cdef, "'__pointer'");

			uint32_t value = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
			if (unlikely(not cdefvalue.isBuiltinU8()))
				return seq.complainIntrinsicParameter("store.u8", 1, cdefvalue, "'__u8'");

			if (seq.canGenerateCode())
				seq.out.emitStoreU8(value, ptrlvid);
			return true;
		}


		static bool intrinsicMemSetPTR(SequenceBuilder& seq, uint32_t lvid)
		{
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
			if (unlikely(not cdef.isRawPointer()))
				return seq.complainIntrinsicParameter("storeptr", 0, cdef, "'__pointer'");

			uint32_t value = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefvalue = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, value});
			if (unlikely(not cdefvalue.isRawPointer()))
				return seq.complainIntrinsicParameter("storeptr", 1, cdefvalue, "'__pointer'");

			if (seq.canGenerateCode())
			{
				if (sizeof(uint64_t) == sizeof(void*))
					seq.out.emitStoreU64(value, ptrlvid);
				else
					seq.out.emitStoreU32(value, ptrlvid);
			}
			return true;
		}


		static bool intrinsicNOT(SequenceBuilder& seq, uint32_t lvid)
		{
			assert(seq.pushedparams.func.indexed.size() == 1);
			uint32_t lhs  = seq.pushedparams.func.indexed[0].lvid;
			auto& cdeflhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, lhs});

			Atom* atomBuiltinCast = nullptr;

			nytype_t builtinlhs = cdeflhs.kind;
			if (builtinlhs == nyt_any) // implicit access to the internal 'pod' variable
			{
				auto* atom = seq.cdeftable.findClassdefAtom(cdeflhs);
				if (atom != nullptr and atom->builtinMapping != nyt_void)
				{
					if (debugmode)
						seq.out.emitComment("reading inner 'pod' variable");
					atomBuiltinCast = atom;
					builtinlhs = atom->builtinMapping;
					uint32_t newlvid = seq.createLocalVariables();
					seq.out.emitFieldget(newlvid, lhs, 0);
					lhs = newlvid;

					if (builtinlhs != nyt_bool) // allow only bool for complex types
						builtinlhs = nyt_any;
				}
			}

			if (unlikely(builtinlhs != nyt_bool and builtinlhs != nyt_ptr))
				return seq.complainIntrinsicParameter("not", 0, cdeflhs);

			// --- result of the operator

			if (atomBuiltinCast != nullptr)
			{
				// implicit convertion from builtin __bool to object bool
				atomBuiltinCast = Atom::Ptr::WeakPointer(seq.cdeftable.atoms().core.object[nyt_bool]);
				assert(atomBuiltinCast != nullptr);
				assert(not atomBuiltinCast->hasGenericParameters());

				Atom* remapAtom = seq.instanciateAtomClass(*atomBuiltinCast);
				if (unlikely(nullptr == remapAtom))
					return false;

				auto& opc = seq.cdeftable.substitute(lvid);
				opc.mutateToAtom(atomBuiltinCast);
				opc.qualifiers.ref = true;

				if (seq.canGenerateCode())
				{
					// creating two variables on the stack
					uint32_t opresult   = seq.createLocalVariables(2);
					uint32_t sizeoflvid = opresult + 1;

					// RESULT: opresult: the first one is the result of the operation (and, +, -...)
					seq.cdeftable.substitute(opresult).mutateToBuiltin(nyt_bool);
					seq.out.emitNOT(opresult, lhs);

					// SIZEOF: the second variable on the stack is `sizeof(<object>)`
					// (sizeof the object to allocate)
					seq.cdeftable.substitute(sizeoflvid).mutateToBuiltin(nyt_u64);
					seq.out.emitSizeof(sizeoflvid, atomBuiltinCast->atomid);

					// ALLOC: memory allocation of the new temporary object
					seq.out.emitMemalloc(lvid, sizeoflvid);
					seq.out.emitRef(lvid);
					seq.frame->lvids[lvid].autorelease = true;
					// reset the internal value of the object
					seq.out.emitFieldset(opresult, /*self*/lvid, 0); // builtin
				}
			}
			else
			{
				// no convertion to perform, direct call
				seq.cdeftable.substitute(lvid).mutateToBuiltin(builtinlhs);
				if (seq.canGenerateCode())
					seq.out.emitNOT(lvid, lhs);
			}
			return true;
		}


		static bool intrinsicAssert(SequenceBuilder& seq, uint32_t lvid)
		{
			assert(seq.pushedparams.func.indexed.size() == 1);

			// no return value
			seq.cdeftable.substitute(lvid).mutateToVoid();

			uint32_t lhs  = seq.pushedparams.func.indexed[0].lvid;
			auto& cdeflhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, lhs});

			Atom* atomBuiltinCast = nullptr;

			nytype_t builtinlhs = cdeflhs.kind;
			if (builtinlhs == nyt_any) // implicit access to the internal 'pod' variable
			{
				auto* atom = seq.cdeftable.findClassdefAtom(cdeflhs);
				if (atom != nullptr and atom->builtinMapping != nyt_void)
				{
					if (debugmode)
						seq.out.emitComment("reading inner 'pod' variable");
					atomBuiltinCast = atom;
					builtinlhs = atom->builtinMapping;

					if (seq.canGenerateCode())
					{
						uint32_t newlvid = seq.createLocalVariables();
						seq.out.emitFieldget(newlvid, lhs, 0);
						lhs = newlvid;
					}
				}
			}

			if (unlikely(builtinlhs != nyt_bool))
				return seq.complainIntrinsicParameter("assert", 0, cdeflhs);

			// --- result of the operator

			if (atomBuiltinCast != nullptr)
			{
				// implicit convertion from builtin __bool to object bool
				atomBuiltinCast = Atom::Ptr::WeakPointer(seq.cdeftable.atoms().core.object[nyt_bool]);
				assert(atomBuiltinCast != nullptr);
				assert(not atomBuiltinCast->hasGenericParameters());

				Atom* remapAtom = seq.instanciateAtomClass(*atomBuiltinCast);
				if (unlikely(nullptr == remapAtom))
					return false;
			}

			if (seq.canGenerateCode())
				seq.out.emitAssert(lhs);
			return true;
		}


		static bool intrinsicReinterpret(SequenceBuilder& seq, uint32_t lvid)
		{
			assert(seq.pushedparams.func.indexed.size() == 2);

			uint32_t lhs    = seq.pushedparams.func.indexed[0].lvid;
			uint32_t tolvid = seq.pushedparams.func.indexed[1].lvid;

			auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, tolvid});

			// copy the type, without any check
			auto& spare = seq.cdeftable.substitute(lvid);
			spare.import(cdef);
			spare.qualifiers = cdef.qualifiers;

			seq.out.emitStore(lvid, lhs);

			auto& lvidinfo = seq.frame->lvids[lvid];
			lvidinfo.synthetic = false;

			if (seq.canBeAcquired(cdef) and seq.canGenerateCode()) // re-acquire the object
			{
				seq.out.emitRef(lvid);
				lvidinfo.autorelease = true;
				lvidinfo.scope = seq.frame->scope;
			}
			return true;
		}





		constexpr static const nytype_t promotion[nyt_count][nyt_count] =
		{
			/*void*/ {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*any*/  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*ptr*/  {nyt_void,nyt_void,nyt_ptr, nyt_void,nyt_ptr, nyt_ptr, nyt_ptr, nyt_ptr, nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*bool*/ {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*u8 */  {nyt_void,nyt_void,nyt_ptr, nyt_void,nyt_u8,  nyt_u16, nyt_u32, nyt_u64, nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*u16*/  {nyt_void,nyt_void,nyt_ptr, nyt_void,nyt_u16, nyt_u16, nyt_u32, nyt_u64, nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*u32*/  {nyt_void,nyt_void,nyt_ptr, nyt_void,nyt_u32, nyt_u32, nyt_u32, nyt_u64, nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*u64*/  {nyt_void,nyt_void,nyt_ptr, nyt_void,nyt_u64, nyt_u64, nyt_u64, nyt_u64, nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,},
			/*i8 */  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_i8,  nyt_i16, nyt_i32, nyt_i64, nyt_void,nyt_void,},
			/*i16*/  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_i16, nyt_i16, nyt_i32, nyt_i64, nyt_void,nyt_void,},
			/*i32*/  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_i32, nyt_i32, nyt_i32, nyt_i64, nyt_void,nyt_void,},
			/*i64*/  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_void,nyt_i64, nyt_i64, nyt_i64, nyt_i64, nyt_void,nyt_void,},
			/*f32*/  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_f32, nyt_f32, nyt_f32, nyt_void,nyt_f32, nyt_f32, nyt_f32, nyt_void,nyt_f32, nyt_f64, },
			/*f64*/  {nyt_void,nyt_void,nyt_void,nyt_void,nyt_f64, nyt_f64, nyt_f64, nyt_f64, nyt_f64, nyt_f64, nyt_f64, nyt_f64, nyt_f64, nyt_f64, },
		};


		template<nytype_t R, bool AcceptBool, bool AcceptInt, bool AcceptFloat,
			void (IR::Sequence::* M)(uint32_t, uint32_t, uint32_t)>
		static inline bool emitBuiltinOperator(SequenceBuilder& seq, uint32_t lvid, const char* const name)
		{
			assert(seq.pushedparams.func.indexed.size() == 2);

			// -- LHS - PARAMETER 0 --
			uint32_t lhs  = seq.pushedparams.func.indexed[0].lvid;
			auto& cdeflhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, lhs});

			Atom* atomBuiltinCast = nullptr;

			nytype_t builtinlhs = cdeflhs.kind;
			if (builtinlhs == nyt_any) // implicit access to the internal 'pod' variable
			{
				auto* atom = seq.cdeftable.findClassdefAtom(cdeflhs);
				if (atom != nullptr and atom->builtinMapping != nyt_void)
				{
					if (debugmode)
						seq.out.emitComment("reading inner 'pod' variable");

					builtinlhs = atom->builtinMapping;
					uint32_t newlvid = seq.createLocalVariables();
					seq.out.emitFieldget(newlvid, lhs, 0);
					lhs = newlvid;
					atomBuiltinCast = atom;
				}
			}

			// -- RHS - PARAMETER 1 --
			uint32_t rhs  = seq.pushedparams.func.indexed[1].lvid;
			auto& cdefrhs = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, rhs});
			nytype_t builtinrhs = cdefrhs.kind;

			if (builtinrhs == nyt_any) // implicit access to the internal 'pod' variable
			{
				auto* atom = seq.cdeftable.findClassdefAtom(cdefrhs);
				if (atom != nullptr and (atom->builtinMapping != nyt_void))
				{
					if (debugmode)
						seq.out.emitComment("reading inner 'pod' variable");

					builtinrhs = atom->builtinMapping;
					uint32_t newlvid = seq.createLocalVariables();
					seq.out.emitFieldget(newlvid, rhs, 0);
					rhs = newlvid;
					atomBuiltinCast = atom;
				}
			}


			// -- implicit type promotion --
			if (builtinlhs != builtinrhs)
			{
				builtinlhs = promotion[builtinlhs][builtinrhs];
				if (unlikely(builtinlhs == nyt_void))
				{
					seq.complainIntrinsicParameter(name, 0, cdeflhs);
					return seq.complainIntrinsicParameter(name, 1, cdefrhs);
				}

				if (atomBuiltinCast != nullptr and builtinlhs != nyt_ptr)
					atomBuiltinCast = Atom::Ptr::WeakPointer(seq.cdeftable.atoms().core.object[builtinlhs]);
			}

			switch (builtinlhs)
			{
				case nyt_bool:
				{
					if (not AcceptBool)
						return seq.complainBuiltinIntrinsicDoesNotAccept(name, "booleans");
					break;
				}
				case nyt_i8:
				case nyt_i16:
				case nyt_i32:
				case nyt_i64:
				case nyt_u8:
				case nyt_u16:
				case nyt_u32:
				case nyt_u64:
				case nyt_ptr:
				{
					if (not AcceptInt)
						return seq.complainBuiltinIntrinsicDoesNotAccept(name, "integer literals");
					break;
				}
				case nyt_f32:
				case nyt_f64:
				{
					if (not AcceptFloat)
						return seq.complainBuiltinIntrinsicDoesNotAccept(name, "floating-point numbers");
					break;
				}
				case nyt_void:
				case nyt_any:
				{
					return seq.complainIntrinsicParameter(name, 0, cdeflhs);
				}
			}

			// --- result of the operator

			nytype_t rettype = (R == nyt_any) ? builtinlhs : R;

			if (rettype != nyt_ptr and atomBuiltinCast != nullptr and seq.shortcircuit.label == 0) // the result is a real instance
			{
				// implicit convertion from builtin (__i32...) to object (i32...)
				if (R != nyt_any) // force the result type
				{
					atomBuiltinCast = Atom::Ptr::WeakPointer(seq.cdeftable.atoms().core.object[R]);
					assert(atomBuiltinCast != nullptr);
					assert(not atomBuiltinCast->hasGenericParameters());
				}

				Atom* remapAtom = seq.instanciateAtomClass(*atomBuiltinCast);
				if (unlikely(nullptr == remapAtom))
					return false;

				auto& opc = seq.cdeftable.substitute(lvid);
				opc.mutateToAtom(atomBuiltinCast);
				opc.qualifiers.ref = true;

				if (seq.canGenerateCode())
				{
					if (debugmode)
						seq.out.emitComment(ShortString32() << "builtin " << name);

					// creating two variables on the stack
					uint32_t opresult   = seq.createLocalVariables(2);
					uint32_t sizeoflvid = opresult + 1;

					// RESULT: opresult: the first one is the result of the operation (and, +, -...)
					seq.cdeftable.substitute(opresult).mutateToBuiltin(rettype);
					(seq.out.*M)(opresult, lhs, rhs);

					// SIZEOF: the second variable on the stack is `sizeof(<object>)`
					// (sizeof the object to allocate)
					seq.cdeftable.substitute(sizeoflvid).mutateToBuiltin(nyt_u64);
					seq.out.emitSizeof(sizeoflvid, atomBuiltinCast->atomid);

					// ALLOC: memory allocation of the new temporary object
					seq.out.emitMemalloc(lvid, sizeoflvid);
					seq.out.emitRef(lvid);
					seq.frame->lvids[lvid].autorelease = true;
					// reset the internal value of the object
					seq.out.emitFieldset(opresult, /*self*/lvid, 0); // builtin
				}
			}
			else
			{
				// no convertion to perform, direct call
				seq.cdeftable.substitute(lvid).mutateToBuiltin(rettype);
				if (seq.canGenerateCode())
				{
					if (debugmode)
						seq.out.emitComment(ShortString32() << "builtin " << name);
					(seq.out.*M)(lvid, lhs, rhs);
				}
			}
			return true;
		}


		static bool intrinsicAND(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitAND>(seq, lvid, "and");
		}

		static bool intrinsicOR(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitOR>(seq, lvid, "or");
		}

		static bool intrinsicXOR(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitXOR>(seq, lvid, "xor");
		}

		static bool intrinsicMOD(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitMOD>(seq, lvid, "mod");
		}

		static bool intrinsicADD(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitADD>(seq, lvid, "add");
		}

		static bool intrinsicSUB(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitSUB>(seq, lvid, "sub");
		}

		static bool intrinsicDIV(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitDIV>(seq, lvid, "div");
		}

		static bool intrinsicMUL(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitMUL>(seq, lvid, "mul");
		}

		static bool intrinsicIDIV(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitIDIV>(seq, lvid, "idiv");
		}

		static bool intrinsicIMUL(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitIMUL>(seq, lvid, "imul");
		}

		static bool intrinsicFADD(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFADD>(seq, lvid, "fadd");
		}

		static bool intrinsicFSUB(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitIGTE>(seq, lvid, "igte");
			return emitBuiltinOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFSUB>(seq, lvid, "fsub");
		}

		static bool intrinsicFDIV(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFDIV>(seq, lvid, "fdiv");
		}

		static bool intrinsicFMUL(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFMUL>(seq, lvid, "fmul");
		}



		static bool intrinsicEQ(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitEQ>(seq, lvid, "eq");
		}

		static bool intrinsicNEQ(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitNEQ>(seq, lvid, "neq");
		}

		static bool intrinsicFLT(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFLT>(seq, lvid, "flt");
		}

		static bool intrinsicFLTE(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFLTE>(seq, lvid, "flte");
		}

		static bool intrinsicFGT(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFGT>(seq, lvid, "fgt");
		}

		static bool intrinsicFGTE(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFGTE>(seq, lvid, "fgte");
		}

		static bool intrinsicLT(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitLT>(seq, lvid, "lt");
		}

		static bool intrinsicLTE(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitLTE>(seq, lvid, "lte");
		}

		static bool intrinsicILT(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitILT>(seq, lvid, "ilt");
		}

		static bool intrinsicILTE(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitILTE>(seq, lvid, "ilte");
		}

		static bool intrinsicGT(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitGT>(seq, lvid, "gt");
		}

		static bool intrinsicGTE(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitGTE>(seq, lvid, "gte");
		}

		static bool intrinsicIGT(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitIGT>(seq, lvid, "igt");
		}

		static bool intrinsicIGTE(SequenceBuilder& seq, uint32_t lvid)
		{
			return emitBuiltinOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitIGTE>(seq, lvid, "igte");
		}












		typedef bool (*BuiltinIntrinsic)(SequenceBuilder&, uint32_t);

		static const std::unordered_map<AnyString, std::pair<uint32_t, BuiltinIntrinsic>> builtinDispatch =
		{
			//
			{"^fieldset",       { 2,  &intrinsicFieldset,    }},
			//
			{"memory.allocate", { 1,  &intrinsicMemalloc,    }},
			{"memory.realloc",  { 3,  &intrinsicMemrealloc,  }},
			{"memory.dispose",  { 2,  &intrinsicMemFree,     }},
			{"memory.fill",     { 3,  &intrinsicMemfill,     }},
			{"memory.copy",     { 3,  &intrinsicMemCopy,     }},
			{"memory.move",     { 3,  &intrinsicMemMove,     }},
			{"memory.cmp",      { 3,  &intrinsicMemCmp,      }},
			{"load.ptr",        { 1,  &intrinsicMemGetPTR,   }},
			{"load.u64",        { 1,  &intrinsicMemGetU64,   }},
			{"load.u32",        { 1,  &intrinsicMemGetU32,   }},
			{"load.u8",         { 1,  &intrinsicMemGetU8,    }},
			{"store.ptr",       { 2,  &intrinsicMemSetPTR,   }},
			{"store.u64",       { 2,  &intrinsicMemSetU64,   }},
			{"store.u32",       { 2,  &intrinsicMemSetU32,   }},
			{"store.u8",        { 2,  &intrinsicMemSetU8,    }},
			//
			{"ref",             { 1,  &intrinsicRef,         }},
			{"unref",           { 1,  &intrinsicUnref,       }},
			{"sizeof",          { 1,  &intrinsicSizeof,      }},
			{"pointer",         { 1,  &intrinsicPointer,     }},
			//
			{"and",             { 2,  &intrinsicAND,         }},
			{"or",              { 2,  &intrinsicOR,          }},
			{"xor",             { 2,  &intrinsicXOR,         }},
			{"mod",             { 2,  &intrinsicMOD,         }},
			//
			{"not",             { 1,  &intrinsicNOT,         }},
			//
			{"add",             { 2,  &intrinsicADD,         }},
			{"sub",             { 2,  &intrinsicSUB,         }},
			{"div",             { 2,  &intrinsicDIV,         }},
			{"mul",             { 2,  &intrinsicMUL,         }},
			{"idiv",            { 2,  &intrinsicIDIV,        }},
			{"imul",            { 2,  &intrinsicIMUL,        }},
			{"fadd",            { 2,  &intrinsicFADD,        }},
			{"fsub",            { 2,  &intrinsicFSUB,        }},
			{"fdiv",            { 2,  &intrinsicFDIV,        }},
			{"fmul",            { 2,  &intrinsicFMUL,        }},
			//
			{"eq",              { 2,  &intrinsicEQ,          }},
			{"neq",             { 2,  &intrinsicNEQ,         }},
			{"flt",             { 2,  &intrinsicFLT,         }},
			{"flte",            { 2,  &intrinsicFLTE,        }},
			{"fgt",             { 2,  &intrinsicFGT,         }},
			{"fgte",            { 2,  &intrinsicFGTE,        }},
			{"lt",              { 2,  &intrinsicLT,          }},
			{"lte",             { 2,  &intrinsicLTE,         }},
			{"ilt",             { 2,  &intrinsicILT,         }},
			{"ilte",            { 2,  &intrinsicILTE,        }},
			{"gt",              { 2,  &intrinsicGT,          }},
			{"gte",             { 2,  &intrinsicGTE,         }},
			{"igt",             { 2,  &intrinsicIGT,         }},
			{"igte",            { 2,  &intrinsicIGTE,        }},
			//
			{"assert",          { 1,  &intrinsicAssert,      }},
			{"__reinterpret",   { 2,  &intrinsicReinterpret, }},
			{"__nanyc_memchecker_hold",  { 2,  &intrinsicMemcheckerHold, }},

			{"os.is.linux",     { 0,  &intrinsicOSIsLinux      }},
			{"os.is.unix",      { 0,  &intrinsicOSIsUnix       }},
			{"os.is.posix",     { 0,  &intrinsicOSIsPosix      }},
			{"os.is.macos",     { 0,  &intrinsicOSIsMacOS      }},
			{"os.is.bsd",       { 0,  &intrinsicOSIsBSD        }},
			{"os.is.aix",       { 0,  &intrinsicOSIsAIX        }},
			{"os.is.windows",   { 0,  &intrinsicOSIsWindows    }},
			{"os.is.cygwin",    { 0,  &intrinsicOSIsCygwin     }},
		};

	} // anonymous namespace




	Yuni::Tribool::Value
	SequenceBuilder::instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid, bool canProduceError)
	{
		assert(not name.empty());
		assert(frame != nullptr);

		if (canProduceError)
		{
			// named parameters are not accepted
			if (unlikely(not pushedparams.func.named.empty()))
			{
				complainIntrinsicWithNamedParameters(name);
				return Tribool::Value::no;
			}

			// generic type parameters are not accepted
			if (unlikely(not pushedparams.gentypes.indexed.empty() or not pushedparams.gentypes.named.empty()))
			{
				complainIntrinsicWithGenTypeParameters(name);
				return Tribool::Value::no;
			}

			// checking if one parameter was already flag as 'error'
			for (uint32_t i = 0u; i != pushedparams.func.indexed.size(); ++i)
			{
				if (unlikely(not frame->verify(pushedparams.func.indexed[i].lvid)))
					return Tribool::Value::no;
			}
		}

		auto it = builtinDispatch.find(name);
		if (unlikely(it == builtinDispatch.end()))
		{
			if (canProduceError)
				complainUnknownIntrinsic(name);
			return Tribool::Value::indeterminate;
		}

		// checking for parameters
		uint32_t count = it->second.first;
		if (unlikely(not checkForIntrinsicParamCount(name, count)))
			return Tribool::Value::no;


		frame->lvids[lvid].synthetic = false;

		// intrinsic builtin found !
		return ((it->second.second))(*this, lvid)
			? Tribool::Value::yes : Tribool::Value::no;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
