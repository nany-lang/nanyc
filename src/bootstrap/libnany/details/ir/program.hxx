#include "program.h"

#define NANY_PRINT_PROGRAM_OPCODES 0
#if NANY_PRINT_PROGRAM_OPCODES != 0
#include <iostream>
#endif



namespace Nany
{
namespace IR
{


	inline void Program::reserve(uint32_t N)
	{
		if (pCapacity < N)
			grow(N);
	}

	inline uint32_t Program::opcodeCount() const
	{
		return pSize;
	}

	inline uint32_t Program::capacity() const
	{
		return pCapacity;
	}


	inline const Instruction& Program::at(uint32_t offset) const
	{
		assert(offset < pSize);
		return pBody.get()[offset];
	}


	inline Instruction& Program::at(uint32_t offset)
	{
		assert(offset < pSize);
		return pBody.get()[offset];
	}


	template<enum ISA::Op O> inline ISA::Operand<O>& Program::at(uint32_t offset)
	{
		assert(offset < pSize);
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		return reinterpret_cast<ISA::Operand<O>&>(pBody.get()[offset]);
	}



	template<enum ISA::Op O> inline const ISA::Operand<O>& Program::at(uint32_t offset) const
	{
		assert(offset < pSize);
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		return reinterpret_cast<ISA::Operand<O>&>(pBody.get()[offset]);
	}


	template<enum ISA::Op O> inline ISA::Operand<O>& Program::emitraw()
	{
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		assert(pSize + 1 < pCapacity);
		auto& result = at<O>(pSize++);
		result.opcode = static_cast<uint32_t>(O);
		return result;
	}


	template<enum ISA::Op O> inline ISA::Operand<O>& Program::emit()
	{
		reserve(pSize + 1);

		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		assert(pSize + 1 < pCapacity);
		auto& result = at<O>(pSize++);
		result.opcode = static_cast<uint32_t>(O);
		return result;
	}


	inline void Program::emitNop()
	{
		emit<ISA::Op::nop>();
	}


	inline void Program::emitRef(uint32_t lvid)
	{
		emit<ISA::Op::ref>().lvid = lvid;
	}


	inline uint32_t Program::emitAllocate(uint32_t lvid, uint32_t atomid)
	{
		auto& operands = emit<ISA::Op::allocate>();
		operands.lvid = lvid;
		operands.atomid = atomid;
		return lvid;
	}


	inline void Program::emitUnref(uint32_t lvid, uint32_t atomid, uint32_t instanceid)
	{
		auto& operands      = emit<ISA::Op::unref>();
		operands.lvid       = lvid;
		operands.atomid     = atomid;
		operands.instanceid = instanceid;
	}


	inline void Program::emitFADD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fadd>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitFSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fsub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitFDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fdiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitFMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fmul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitADD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::add>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::sub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::div>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::mul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitIMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::imul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitIDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::idiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Program::emitEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::eq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitNEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::neq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Program::emitFLT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::flt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitFLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::flte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitFGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fgt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitFGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fgte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Program::emitLT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::lt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::lte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitILT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::ilt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitILTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::ilte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::gt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::gte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitIGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::igt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitIGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::igte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Program::emitAND(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opand>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitOR(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitXOR(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opxor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Program::emitMOD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opmod>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Program::emitTypeIsObject(uint32_t lvid)
	{
		emit<ISA::Op::typeisobject>().lvid = lvid;
	}


	inline void Program::emitInheritQualifiers(uint32_t lhs, uint32_t rhs)
	{
		auto& operands   = emit<ISA::Op::inherit>();
		operands.inherit = 2;
		operands.lhs     = lhs;
		operands.rhs     = rhs;
	}


	inline void Program::emitPragmaSuggest(bool onoff)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::suggest);
		opc.value.suggest = static_cast<uint32_t>(onoff);
	}

	inline void Program::emitPragmaBuiltinAlias(const AnyString& name)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::builtinalias);
		opc.value.builtinalias.namesid = stringrefs.ref(name);
	}


	inline void Program::emitPragmaShortcircuit(bool evalvalue)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::shortcircuit);
		opc.value.shortcircuit = static_cast<uint32_t>(evalvalue);
	}

	inline void Program::emitPragmaShortcircuitMetadata(uint32_t label)
	{
		auto& opc = emit<ISA::Op::pragma>();
		opc.pragma = static_cast<uint32_t>(IR::ISA::Pragma::shortcircuitOpNopOffset);
		opc.value.shortcircuitMetadata.label = label;
	}


	inline void Program::emitLabel(uint32_t labelid)
	{
		emit<IR::ISA::Op::label>().label = labelid;
	}


	inline void Program::emitPragmaFuncBody()
	{
		emit<ISA::Op::pragma>().pragma = static_cast<uint32_t>(IR::ISA::Pragma::bodystart);
	}


	inline void Program::emitPragmaAllowCodeGeneration(bool enabled)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::codegen;
		operands.value.codegen = static_cast<uint32_t>(enabled);
	}

	inline void Program::emitVisibility(nyvisibility_t visibility)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::visibility;
		operands.value.visibility = static_cast<uint32_t>(visibility);
	}

	inline uint32_t Program::emitStackSizeIncrease(uint32_t size)
	{
		uint32_t offset = pSize;
		emit<ISA::Op::stacksize>().add = size;
		return offset;
	}


	inline uint32_t Program::emitStackSizeIncrease()
	{
		return emitStackSizeIncrease(0);
	}

	inline uint32_t Program::emitBlueprintSize()
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintsize;
		operands.value.blueprintsize = 0;
		return offset;
	}


	inline void Program::emitBlueprintClass(const AnyString& name, uint32_t atomid)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintclassdef;
		operands.value.blueprint.name   = stringrefs.ref(name);
		operands.value.blueprint.atomid = atomid;
	}

	inline uint32_t Program::emitBlueprintClass()
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintclassdef;
		operands.value.blueprint.name   = 0;;
		operands.value.blueprint.atomid = (uint32_t) -1;
		return offset;
	}

	inline void Program::emitBlueprintFunc(const AnyString& name, uint32_t atomid)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintfuncdef;
		operands.value.blueprint.name   = stringrefs.ref(name);
		operands.value.blueprint.atomid = atomid;
	}

	inline uint32_t Program::emitBlueprintFunc()
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintfuncdef;
		operands.value.blueprint.name   = 0;
		operands.value.blueprint.atomid = (uint32_t) -1;
		return offset;
	}

	inline uint32_t Program::emitBlueprintParam(LVID lvid, const AnyString& name)
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintparam;
		operands.value.param.name = stringrefs.ref(name);
		operands.value.param.lvid = lvid;
		return offset;
	}

	inline uint32_t Program::emitBlueprintParam(LVID lvid)
	{
		uint32_t offset = pSize;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintparam;
		operands.value.param.name = 0;
		operands.value.param.lvid = lvid;
		return offset;
	}


	inline void Program::emitBlueprintVardef(LVID lvid, const AnyString& name)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::blueprintvar;
		operands.value.vardef.name = stringrefs.ref(name);
		operands.value.vardef.lvid = lvid;
	}


	inline void Program::emitStore_u64(uint32_t lvid, uint64_t value)
	{
		auto& operands = emit<ISA::Op::storeConstant>();
		operands.lvid  = lvid;
		operands.value.u64 = value;
	}

	inline void Program::emitStore_f64(uint32_t lvid, double value)
	{
		auto& operands = emit<ISA::Op::storeConstant>();
		operands.lvid  = lvid;
		operands.value.f64 = value;
	}


	inline uint32_t Program::emitStoreText(uint32_t lvid, const AnyString& text)
	{
		auto& operands = emit<ISA::Op::storeText>();
		operands.lvid = lvid;
		operands.text = stringrefs.ref(text);
		return operands.text;
	}


	inline uint32_t Program::emitStackalloc(uint32_t lvid, nytype_t type)
	{
		auto& operands = emit<ISA::Op::stackalloc>();
		operands.lvid   = lvid;
		operands.type   = static_cast<uint32_t>(type);
		operands.atomid = (uint32_t) -1;
		return lvid;
	}


	inline uint32_t Program::emitStackalloc_u64(uint32_t lvid, nytype_t type, uint64_t value)
	{
		emitStackalloc(lvid, type);
		emitStore_u64(lvid, value);
		return lvid;
	}

	inline uint32_t Program::emitStackalloc_f64(uint32_t lvid, nytype_t type, double value)
	{
		emitStackalloc(lvid, type);
		emitStore_f64(lvid, value);
		return lvid;
	}


	inline uint32_t Program::emitStackallocText(uint32_t lvid, const AnyString& text)
	{
		emitStackalloc(lvid, nyt_pointer);
		emitStoreText(lvid, text);
		return lvid;
	}

	inline void Program::emitStore(uint32_t lvid, uint32_t source)
	{
		auto& operands  = emit<ISA::Op::store>();
		operands.lvid   = lvid;
		operands.source = source;
	}


	inline void Program::emitNamespace(const AnyString& name)
	{
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = (uint32_t) ISA::Pragma::namespacedef;
		operands.value.namespacedef = stringrefs.ref(name);
	}


	inline void Program::emitDebugpos(uint32_t line, uint32_t offset)
	{
		auto& operands  = emit<ISA::Op::debugpos>();
		operands.line   = line;
		operands.offset = offset;
	}


	inline void Program::emitPush(uint32_t lvid)
	{
		auto& operands = emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = 0;
	}


	inline uint32_t Program::emitMemalloc(uint32_t lvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memalloc>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
		return lvid;
	}

	inline void Program::emitMemFree(uint32_t lvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memfree>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
	}


	inline void Program::emitSizeof(uint32_t lvid, uint32_t type)
	{
		auto& operands = emit<ISA::Op::classdefsizeof>();
		operands.lvid  = lvid;
		operands.type  = type;
	}


	inline void Program::emitNameAlias(uint32_t lvid, const AnyString& name)
	{
		auto& operands = emit<ISA::Op::namealias>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}


	inline void Program::emitReturn(uint32_t lvid)
	{
		emit<ISA::Op::ret>().lvid = lvid;
	}


	inline void Program::emitComment(const AnyString& text)
	{
		emit<ISA::Op::comment>().text = stringrefs.ref(text);
	}


	inline void Program::emitComment()
	{
		emit<ISA::Op::comment>().text = 0;
	}



	inline void Program::emitSelf(uint32_t self)
	{
		emit<ISA::Op::self>().self = self;
	}


	inline void Program::emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS)
	{
		auto& operands = emit<ISA::Op::assign>();
		operands.lhs = lhs;
		operands.rhs = rhs;
		operands.disposelhs = canDisposeLHS;
	}


	inline void Program::emitIdentify(uint32_t lvid, const AnyString& name, uint32_t self)
	{
		auto& operands = emit<ISA::Op::identify>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.text  = stringrefs.ref(name);
	}


	inline void Program::emitPush(uint32_t lvid, const AnyString& name)
	{
		assert(not name.empty());
		auto& operands = emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}


	inline void Program::emitCall(uint32_t lvid, uint32_t ptr2func)
	{
		auto& operands = emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = ptr2func;
		operands.instanceid = (uint32_t) -1;
	}

	inline void Program::emitCall(uint32_t lvid, uint32_t atomid, uint32_t instanceid)
	{
		assert(instanceid != (uint32_t) -1);
		auto& operands = emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = atomid;
		operands.instanceid = instanceid;
	}


	inline void Program::emitIntrinsic(uint32_t lvid, const AnyString& name)
	{
		auto& operands     = emit<ISA::Op::intrinsic>();
		operands.lvid      = lvid;
		operands.intrinsic = stringrefs.ref(name);
	}


	inline void Program::emitFieldget(uint32_t lvid, uint32_t self, uint32_t fieldindex)
	{
		auto& operands = emit<ISA::Op::fieldget>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = fieldindex;
	}


	inline void Program::emitFieldset(uint32_t lvid, uint32_t self, uint32_t varid)
	{
		auto& operands = emit<ISA::Op::fieldset>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = varid;
	}


	inline void Program::emitScope()
	{
		emit<ISA::Op::scope>();
	}


	inline void Program::emitEnd()
	{
		emit<ISA::Op::end>();
	}


	inline void Program::emitJmp(uint32_t label)
	{
		emit<ISA::Op::jmp>().label = label;
	}

	inline void Program::emitJz(uint32_t lvid, uint32_t result, uint32_t label)
	{
		auto& opc  = emit<ISA::Op::jz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}

	inline void Program::emitJnz(uint32_t lvid, uint32_t result, uint32_t label)
	{
		auto& opc  = emit<ISA::Op::jnz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}


	inline void Program::emitQualifierRef(uint32_t lvid, bool flag)
	{
		auto& operands = emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = 1; // ref
	}


	inline void Program::emitQualifierConst(uint32_t lvid, bool flag)
	{
		auto& operands = emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = 2; // const
	}



	inline const void* Program::pointer(uint32_t offset) const
	{
		return reinterpret_cast<const void*>(pBody.get() + offset);
	}


	inline Yuni::String Program::gdbMemoryWatch(uint32_t offset) const
	{
		return YString{} << "awatch (char[" << sizeof(Instruction) << "]) *" << pointer(offset);
	}


	inline uint32_t Program::offsetOf(const Instruction& instr) const
	{
		assert(pSize > 0 and pCapacity > 0);
		assert(&instr >= pBody.get());
		assert(&instr <  pBody.get() + pSize);

		auto start = reinterpret_cast<std::uintptr_t>(pBody.get());
		auto end   = reinterpret_cast<std::uintptr_t>(&instr);
		assert((end - start) / sizeof(Instruction) < 512 * 1024 * 1024); // arbitrary

		uint32_t r = static_cast<uint32_t>(((end - start) / sizeof(Instruction)));
		assert(r < pSize);
		return r;
	}

	template<enum ISA::Op O>
	inline uint32_t Program::offsetOf(const ISA::Operand<O>& instr) const
	{
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "pSize mismatch");
		return offsetOf(reinterpret_cast<const Instruction&>(instr));
	}


	inline bool Program::hasAtomParent() const
	{
		return (nullptr != pAtom);
	}


	inline void Program::invalidateCursor(const Instruction*& cursor) const
	{
		cursor = pBody.get() + pSize;
	}

	inline void Program::invalidateCursor(Instruction*& cursor) const
	{
		cursor = pBody.get() + pSize;
	}


	inline void Program::jumpToLabelForward(const Instruction*& cursor, uint32_t label) const
	{
		auto* end = pBody.get() + pSize;
		while (++cursor < end)
		{
			if (cursor->opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::label))
			{
				auto& operands = reinterpret_cast<const IR::ISA::Operand<IR::ISA::Op::label>&>(*cursor);
				if (operands.label == label)
					return;
			}
		}
		// not found - the cursor is alreayd invalidated
	}


	inline void Program::jumpToLabelBackward(const Instruction*& cursor, uint32_t label) const
	{
		auto* base = pBody.get();
		while (cursor-- > base)
		{
			if (cursor->opcodes[0] == static_cast<uint32_t>(IR::ISA::Op::label))
			{
				auto& operands = reinterpret_cast<const IR::ISA::Operand<IR::ISA::Op::label>&>(*cursor);
				if (operands.label == label)
					return;
			}
		}
		// not found - invalidate
		invalidateCursor(cursor);
	}


	template<class T> inline void Program::each(T& visitor, uint32_t offset)
	{
		if (likely(offset < pSize))
		{
			auto* it = pBody.get() + offset;
			auto* end = pBody.get() + pSize;
			visitor.cursor = &it;
			for ( ; it < end; ++it)
			{
				#if NANY_PRINT_PROGRAM_OPCODES != 0
				std::cout << "== opcode == at " << (it - pBody.get()) << "|" << (void*) it << " :: "
					<< it->opcodes[0] << ": " << IR::ISA::print(*this, *it) << '\n';
				#endif
				LIBNANY_IR_VISIT_PROGRAM(IR::ISA::Operand, visitor, *it);
			}
		}
	}


	template<class T> inline void Program::each(T& visitor, uint32_t offset) const
	{
		if (likely(offset < pSize))
		{
			const auto* it = pBody.get() + offset;
			const auto* end = pBody.get() + pSize;
			visitor.cursor = &it;
			for ( ; it < end; ++it)
			{
				#if NANY_PRINT_PROGRAM_OPCODES != 0
				std::cout << "== opcode == at " << (it - pBody.get()) << "|" << (void*) it << " :: "
					<< it->opcodes[0] << ": " << IR::ISA::print(*this, *it) << '\n';
				#endif
				LIBNANY_IR_VISIT_PROGRAM(const IR::ISA::Operand, visitor, *it);
			}
		}
	}






} // namespace IR
} // namespace Nany
