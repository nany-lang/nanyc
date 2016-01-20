#include "instanciate.h"
#include "details/type/type-check.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	bool  SequenceBuilder::instanciateIntrinsicFieldset(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();

		auto& frame = atomStack.back();
		uint32_t lvidsid = lastPushedIndexedParameters[1].lvid;
		uint32_t sid = frame.lvids[lvidsid].text_sid;
		if (unlikely(sid == (uint32_t) -1))
			return (ICE() << "invalid string-id for field name (got lvid " << lvidsid << ')');

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


	bool  SequenceBuilder::instanciateIntrinsicRef(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();
		if (canGenerateCode())
			tryToAcquireObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	bool  SequenceBuilder::instanciateIntrinsicUnref(uint32_t lvid)
	{
		cdeftable.substitute(lvid).mutateToVoid();
		if (canGenerateCode())
			tryUnrefObject(lastPushedIndexedParameters[0].lvid);
		return true;
	}


	bool  SequenceBuilder::instanciateIntrinsicAddressof(uint32_t lvid)
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


	bool  SequenceBuilder::instanciateIntrinsicSizeof(uint32_t lvid)
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


	bool  SequenceBuilder::instanciateIntrinsicMemalloc(uint32_t lvid)
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


	bool  SequenceBuilder::instanciateIntrinsicMemFree(uint32_t lvid)
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


	template<nytype_t R, bool AcceptBool, bool AcceptInt, bool AcceptFloat,
		void (IR::Sequence::* M)(uint32_t, uint32_t, uint32_t)>
	inline bool SequenceBuilder::instanciateIntrinsicOperator(uint32_t lvid, const char* const name)
	{
		assert(lastPushedIndexedParameters.size() == 2);
		auto& frame = atomStack.back();
		uint32_t lhs  = lastPushedIndexedParameters[0].lvid;
		auto& cdeflhs   = cdeftable.classdefFollowClassMember(CLID{frame.atomid, lhs});

		Atom* atomBuiltinCast = nullptr;

		nytype_t builtinlhs = cdeflhs.kind;
		if (builtinlhs == nyt_any)
		{
			auto* atom = cdeftable.findClassdefAtom(cdeflhs);
			if (atom != nullptr and atom->builtinMapping != nyt_void)
			{
				atomBuiltinCast = atom;
				builtinlhs = atom->builtinMapping;
				uint32_t newlvid = createLocalVariables();
				out.emitFieldget(newlvid, lhs, 0);
				lhs = newlvid;
			}
		}

		uint32_t rhs  = lastPushedIndexedParameters[1].lvid;
		auto& cdefrhs = cdeftable.classdefFollowClassMember(CLID{frame.atomid, rhs});
		nytype_t builtinrhs = cdefrhs.kind;

		if (builtinrhs == nyt_any)
		{
			auto* atom = cdeftable.findClassdefAtom(cdefrhs);
			if (atom != nullptr and (atom->builtinMapping != nyt_void))
			{
				atomBuiltinCast = atom;
				builtinrhs = atom->builtinMapping;
				uint32_t newlvid = createLocalVariables();
				out.emitFieldget(newlvid, rhs, 0);
				rhs = newlvid;
			}
		}

		if (unlikely(builtinlhs != builtinrhs))
		{
			complainIntrinsicParameter(name, 0, cdeflhs);
			return complainIntrinsicParameter(name, 1, cdefrhs);
		}

		switch (builtinlhs)
		{
			case nyt_bool:
			{
				if (not AcceptBool)
					return complainBuiltinIntrinsicDoesNotAccept(name, "booleans");
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
			{
				if (not AcceptInt)
					return complainBuiltinIntrinsicDoesNotAccept(name, "integer literals");
				break;
			}
			case nyt_f32:
			case nyt_f64:
			{
				if (not AcceptFloat)
					return complainBuiltinIntrinsicDoesNotAccept(name, "floating-point numbers");
				break;
			}
			case nyt_void:
			case nyt_any:
			case nyt_pointer:
			case nyt_count:
			{
				return complainIntrinsicParameter(name, 0, cdeflhs);
			}
		}

		// --- result of the operator

		nytype_t rettype = (R == nyt_any) ? builtinlhs : R;

		if (atomBuiltinCast != nullptr)
		{
			// implicit convertion from builtin (__i32...) to object (i32...)
			if (R != nyt_any) // force the result type
			{
				atomBuiltinCast = Atom::Ptr::WeakPointer(cdeftable.atoms().core.object[R]);
				assert(atomBuiltinCast != nullptr);
			}

			if (unlikely(not instanciateAtomClass(*atomBuiltinCast)))
				return false;

			auto& opc = cdeftable.substitute(lvid);
			opc.mutateToAtom(atomBuiltinCast);
			opc.qualifiers.ref = true;

			if (canGenerateCode())
			{
				// creating two variables on the stack
				uint32_t opresult   = createLocalVariables(2);
				uint32_t sizeoflvid = opresult + 1;

				// RESULT: opresult: the first one is the result of the operation (and, +, -...)
				cdeftable.substitute(opresult).mutateToBuiltin(rettype);
				(out.*M)(opresult, lhs, rhs);

				// SIZEOF: the second variable on the stack is `sizeof(<object>)`
				// (sizeof the object to allocate)
				cdeftable.substitute(sizeoflvid).mutateToBuiltin(nyt_u64);
				out.emitSizeof(sizeoflvid, atomBuiltinCast->atomid);

				// ALLOC: memory allocation of the new temporary object
				out.emitMemalloc(lvid, sizeoflvid);
				out.emitRef(lvid);
				atomStack.back().lvids[lvid].autorelease = true;
				// reset the internal value of the object
				out.emitFieldset(opresult, /*self*/lvid, 0); // builtin
			}
		}
		else
		{
			// no convertion to perform, direct call
			cdeftable.substitute(lvid).mutateToBuiltin(rettype);
			if (canGenerateCode())
				(out.*M)(lvid, lhs, rhs);
		}
		return true;
	}


	bool  SequenceBuilder::instanciateIntrinsicAND(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitAND>(lvid, "and");
	}

	bool  SequenceBuilder::instanciateIntrinsicOR(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitOR>(lvid, "or");
	}

	bool  SequenceBuilder::instanciateIntrinsicXOR(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitXOR>(lvid, "xor");
	}

	bool  SequenceBuilder::instanciateIntrinsicMOD(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 1, 1, 0, &IR::Sequence::emitMOD>(lvid, "mod");
	}

	bool  SequenceBuilder::instanciateIntrinsicADD(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitADD>(lvid, "add");
	}

	bool  SequenceBuilder::instanciateIntrinsicSUB(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitSUB>(lvid, "sub");
	}

	bool  SequenceBuilder::instanciateIntrinsicDIV(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitDIV>(lvid, "div");
	}

	bool  SequenceBuilder::instanciateIntrinsicMUL(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitMUL>(lvid, "mul");
	}

	bool  SequenceBuilder::instanciateIntrinsicIDIV(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitIDIV>(lvid, "idiv");
	}

	bool  SequenceBuilder::instanciateIntrinsicIMUL(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 1, 0, &IR::Sequence::emitIMUL>(lvid, "imul");
	}

	bool  SequenceBuilder::instanciateIntrinsicFADD(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFADD>(lvid, "fadd");
	}

	bool  SequenceBuilder::instanciateIntrinsicFSUB(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFSUB>(lvid, "fsub");
	}

	bool  SequenceBuilder::instanciateIntrinsicFDIV(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFDIV>(lvid, "fdiv");
	}

	bool  SequenceBuilder::instanciateIntrinsicFMUL(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_any, 0, 0, 1, &IR::Sequence::emitFMUL>(lvid, "fmul");
	}



	bool  SequenceBuilder::instanciateIntrinsicEQ(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitEQ>(lvid, "eq");
	}

	bool  SequenceBuilder::instanciateIntrinsicNEQ(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitNEQ>(lvid, "neq");
	}

	bool  SequenceBuilder::instanciateIntrinsicFLT(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFLT>(lvid, "flt");
	}

	bool  SequenceBuilder::instanciateIntrinsicFLTE(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFLTE>(lvid, "flte");
	}

	bool  SequenceBuilder::instanciateIntrinsicFGT(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFGT>(lvid, "fgt");
	}

	bool  SequenceBuilder::instanciateIntrinsicFGTE(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitFGTE>(lvid, "fgte");
	}

	bool  SequenceBuilder::instanciateIntrinsicLT(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitLT>(lvid, "lt");
	}

	bool  SequenceBuilder::instanciateIntrinsicLTE(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitLTE>(lvid, "lte");
	}

	bool  SequenceBuilder::instanciateIntrinsicILT(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitILT>(lvid, "ilt");
	}

	bool  SequenceBuilder::instanciateIntrinsicILTE(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitILTE>(lvid, "ilte");
	}

	bool  SequenceBuilder::instanciateIntrinsicGT(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitGT>(lvid, "gt");
	}

	bool  SequenceBuilder::instanciateIntrinsicGTE(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitGTE>(lvid, "gte");
	}

	bool  SequenceBuilder::instanciateIntrinsicIGT(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitIGT>(lvid, "igt");
	}

	bool  SequenceBuilder::instanciateIntrinsicIGTE(uint32_t lvid)
	{
		return instanciateIntrinsicOperator<nyt_bool, 1, 1, 1, &IR::Sequence::emitIGTE>(lvid, "igte");
	}














	typedef bool (SequenceBuilder::* BuiltinIntrinsic)(uint32_t);

	static const std::unordered_map<AnyString, std::pair<BuiltinIntrinsic, uint32_t>> builtinDispatch =
	{
		{"^fieldset",       { &SequenceBuilder::instanciateIntrinsicFieldset,  2 }},
		{"addressof",       { &SequenceBuilder::instanciateIntrinsicAddressof, 1 }},
		{"memory.allocate", { &SequenceBuilder::instanciateIntrinsicMemalloc,  1 }},
		{"memory.dispose",  { &SequenceBuilder::instanciateIntrinsicMemFree,   2 }},
		{"ref",             { &SequenceBuilder::instanciateIntrinsicRef,       1 }},
		{"unref",           { &SequenceBuilder::instanciateIntrinsicUnref,     1 }},
		{"sizeof",          { &SequenceBuilder::instanciateIntrinsicSizeof,    1 }},
		//
		{"and",             { &SequenceBuilder::instanciateIntrinsicAND,       2 }},
		{"or",              { &SequenceBuilder::instanciateIntrinsicOR,        2 }},
		{"xor",             { &SequenceBuilder::instanciateIntrinsicXOR,       2 }},
		{"mod",             { &SequenceBuilder::instanciateIntrinsicMOD,       2 }},
		//
		{"add",             { &SequenceBuilder::instanciateIntrinsicADD,       2 }},
		{"sub",             { &SequenceBuilder::instanciateIntrinsicSUB,       2 }},
		{"div",             { &SequenceBuilder::instanciateIntrinsicDIV,       2 }},
		{"mul",             { &SequenceBuilder::instanciateIntrinsicMUL,       2 }},
		{"idiv",            { &SequenceBuilder::instanciateIntrinsicIDIV,      2 }},
		{"imul",            { &SequenceBuilder::instanciateIntrinsicIMUL,      2 }},
		{"fadd",            { &SequenceBuilder::instanciateIntrinsicFADD,      2 }},
		{"fsub",            { &SequenceBuilder::instanciateIntrinsicFSUB,      2 }},
		{"fdiv",            { &SequenceBuilder::instanciateIntrinsicFDIV,      2 }},
		{"fmul",            { &SequenceBuilder::instanciateIntrinsicFMUL,      2 }},
		//
		{"eq",              { &SequenceBuilder::instanciateIntrinsicEQ,        2 }},
		{"neq",             { &SequenceBuilder::instanciateIntrinsicNEQ,       2 }},
		{"flt",             { &SequenceBuilder::instanciateIntrinsicFLT,       2 }},
		{"flte",            { &SequenceBuilder::instanciateIntrinsicFLTE,      2 }},
		{"fgt",             { &SequenceBuilder::instanciateIntrinsicFGT,       2 }},
		{"fgte",            { &SequenceBuilder::instanciateIntrinsicFGTE,      2 }},
		{"lt",              { &SequenceBuilder::instanciateIntrinsicLT,        2 }},
		{"lte",             { &SequenceBuilder::instanciateIntrinsicLTE,       2 }},
		{"ilt",             { &SequenceBuilder::instanciateIntrinsicILT,       2 }},
		{"ilte",            { &SequenceBuilder::instanciateIntrinsicILTE,      2 }},
		{"gt",              { &SequenceBuilder::instanciateIntrinsicGT,        2 }},
		{"gte",             { &SequenceBuilder::instanciateIntrinsicGTE,       2 }},
		{"igt",             { &SequenceBuilder::instanciateIntrinsicIGT,       2 }},
		{"igte",            { &SequenceBuilder::instanciateIntrinsicIGTE,      2 }},
	};

	bool SequenceBuilder::instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid, bool canComplain)
	{
		assert(not name.empty());

		bool success = ([&]() -> bool
		{
			auto it = builtinDispatch.find(name);
			if (unlikely(it == builtinDispatch.end()))
				return (canComplain and complainUnknownIntrinsic(name));

			if (unlikely(not lastPushedNamedParameters.empty()))
				return complainIntrinsicWithNamedParameters(name);


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
			return (this->*(it->second.first))(lvid);
		})();

		// annotate any error
		if (unlikely(not success and canComplain))
			atomStack.back().lvids[lvid].errorReported = true;
		return success;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
