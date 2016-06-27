#include "vm.h"
#include "details/vm/types.h"
#include "stack.h"
#include "stacktrace.h"
#include "memchecker.h"
#include "dyncall/dyncall.h"
#include "details/ir/isa/data.h"
#include <iostream>
#include <csetjmp>
#include <limits>


using namespace Yuni;



//! Print opcodes executed by the vm
#define NANY_VM_PRINT_OPCODES 0



#define VM_CHECK_POINTER(P,LVID)  if (YUNI_UNLIKELY(not memchecker.has((P)))) \
	{ \
		emitUnknownPointer((P)); /*assert(false and "invalid pointer");*/ \
	}



#if NANY_VM_PRINT_OPCODES != 0
#define VM_PRINT_OPCODE(O)  do { std::cout << "== nany:vm ==  " \
	<< Nany::IR::ISA::print(sequence.get(), operands, &map) << '\n';} while (0)
#else
#define VM_PRINT_OPCODE(O)
#endif






namespace Nany
{
namespace VM
{

	//! Pattern for memset alloc regions (debug)
	constexpr static const int patternAlloc = 0xCD;

	//! Pattern for memset free regions (debug)
	constexpr static const int patternFree = 0xCD;



	namespace // anonymous
	{

		struct Executor final
		{
			nyallocator_t& allocator;

			DCCallVM* dyncall = nullptr;

			//! For C calls
			nyvm_t cfvm;

			nyprogram_cf_t cf;

			ThreadContext& threadContext;

			//! Registers for the current stack frame
			DataRegister* registers = nullptr;
			//! Return value
			uint64_t retRegister = 0;

			//! Number of pushed parameters
			uint32_t funcparamCount = 0; // parameters are 2-based
			//! all pushed parameters
			DataRegister funcparams[Config::maxPushedParameters];

			Stack stack;

			//! Stack trace
			Stacktrace<true> stacktrace;
			//! Memory checker
			MemChecker<true> memchecker;

			#ifndef NDEBUG
			//! Total number of registers in the current frame
			uint32_t registerCount = 0;
			#endif

			//! upper label id encountered so far
			uint32_t upperLabelID = 0;
			//! Atom collection references
			const AtomMap& map;

			//! Source sequence
			std::reference_wrapper<const IR::Sequence> sequence;

			//! All user-defined intrinsics
			const IntrinsicTable& userDefinedIntrinsics;


			//! Reference to the current iterator
			const IR::Instruction** cursor = nullptr;
			//! Jump buffer, to handle exceptions during the execution of the program
			std::jmp_buf jump_buffer;


		public:
			Executor(ThreadContext& threadContext, const IR::Sequence& callee)
				: allocator(threadContext.program.cf.allocator)
				, cf(threadContext.program.cf)
				, threadContext(threadContext)
				, stack(Nany::ref(threadContext.program.build))
				, map(threadContext.program.map)
				, sequence(std::cref(callee))
				, userDefinedIntrinsics(Nany::ref(threadContext.program.build).intrinsics)
			{
				// dynamic C calls
				dyncall = dcNewCallVM(4096);
				dcMode(dyncall, DC_CALL_C_DEFAULT);

				// prepare the current context for native C calls
				cfvm.allocator = &allocator;
				cfvm.program = threadContext.program.self();
				cfvm.tctx = threadContext.self();
				cfvm.console = &cf.console;
			}


			~Executor()
			{
				// releasing dyncall object
				if (dyncall)
					dcFree(dyncall);
				// memory leaks ?
				memchecker.printLeaksIfAny(threadContext.cf);
			}

			[[noreturn]] void abortMission()
			{
				// dump information on the console
				stacktrace.dump(Nany::ref(threadContext.program.build), map);

				// avoid spurious err messages and memory leaks when used
				// as scripting engine
				memchecker.releaseAll(allocator);
				// abort the execution of the program
				std::longjmp(jump_buffer, 666);
			}

			[[noreturn]] void emitBadAlloc()
			{
				threadContext.cerrException("failed to allocate memory");
				abortMission();
			}

			[[noreturn]] void emitPointerSizeMismatch(void* object, size_t size)
			{
				ShortString64 msg;
				msg << "pointer " << (void*) object << " size mismatch: got " << size;
				threadContext.cerrException(msg);
				abortMission();
			}
			[[noreturn]] void emitAssert()
			{
				threadContext.cerrException("assertion failed");
				abortMission();
			}
			[[noreturn]] void emitUnexpectedOpcode(const AnyString& name)
			{
				ShortString64 msg;
				msg << "error: unexpected opcode '" << name << '\'';
				threadContext.cerrException(msg);
				abortMission();
			}
			[[noreturn]] void emitInvalidIntrinsicParamType()
			{
				threadContext.cerrException("intrinsic invalid parameter type");
				abortMission();
			}
			[[noreturn]] void emitInvalidReturnType()
			{
				threadContext.cerrException("intrinsic invalid return type");
				abortMission();
			}
			[[noreturn]] void emitDividedByZero()
			{
				threadContext.cerrException("division by zero");
				abortMission();
			}
			[[noreturn]] void emitUnknownPointer(void* p)
			{
				ShortString256 msg;
				msg << "unknown pointer " << p << ", opcode: +";
				msg << sequence.get().offsetOf(**cursor);
				threadContext.cerrException(msg);
				abortMission();
			}
			[[noreturn]] void emitLabelError(uint32_t label)
			{
				ShortString256 msg;
				msg << "invalid label %" << label << " (upper: %" << upperLabelID;
				msg << "), opcode: +" << sequence.get().offsetOf(**cursor);
				threadContext.cerrException(msg);
				abortMission();
			}


			/*!
			** \brief Adjust an input size for extra metadata held by an object
			*/
			static constexpr inline size_t extraObjectSize()
			{
				return sizeof(uint64_t); // object ref counter
			}


			template<class T> inline T* allocateraw(size_t size)
			{
				return (T*) allocator.allocate(&allocator, size);
			}


			inline void deallocate(void* object, size_t size)
			{
				assert(object != nullptr);
				allocator.deallocate(&allocator, object, size);
			}




			void destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid)
			{
				// the dtor function to call
				const Atom* dtor = map.findAtom(dtorid);
				assert(dtor != nullptr);

				if (false) // traces
				{
					std::cout << " .. DESTROY " << (void*) object << " aka '"
						<< dtor->caption() << "' at opc+" << sequence.get().offsetOf(**cursor) << '\n';
					stacktrace.dump(Nany::ref(threadContext.program.build), map);
					std::cout << '\n';
				}

				// the parent class
				const Atom* classobject = dtor->parent;
				assert(classobject != nullptr);

				// its size
				uint64_t classsizeof = classobject->runtimeSizeof();
				classsizeof += extraObjectSize();

				if (instanceid != (uint32_t) -1)
				{
					// reset parameters for func call
					funcparamCount = 1;
					funcparams[0].u64 = reinterpret_cast<uint64_t>(object); // self
					// func call
					call(0, dtor->atomid, instanceid);
				}

				if (debugmode)
					memset(object, patternFree, classsizeof);

				// sandbox release
				deallocate(object, static_cast<size_t>(classsizeof));
				memchecker.forget(object);
			}



			inline void gotoLabel(uint32_t label)
			{
				bool jmpsuccess = (label > upperLabelID)
					? sequence.get().jumpToLabelForward(*cursor, label)
					: sequence.get().jumpToLabelBackward(*cursor, label);
				if (unlikely(not jmpsuccess))
					emitLabelError(label);
			}



			// accept those opcode for debugging purposes
			inline void visit(const IR::ISA::Operand<IR::ISA::Op::comment>&)
			{
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::scope>&)
			{
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::end>&)
			{
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::nop>&)
			{
			}




			inline void visit(const IR::ISA::Operand<IR::ISA::Op::negation>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount);
				registers[opr.lvid].u64 = not registers[opr.lhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& opr)
			{
				VM_PRINT_OPCODE(operands);
				dcReset(dyncall);
				dcArgPointer(dyncall, &cfvm);

				auto& intrinsic = userDefinedIntrinsics[opr.iid];
				for (uint32_t i = 0; i != funcparamCount; ++i)
				{
					auto r = funcparams[i];
					switch (intrinsic.params[i])
					{
						case nyt_u64:
							dcArgLongLong(dyncall, static_cast<DClonglong>(r.u64));
							break;
						case nyt_i64:
							dcArgLongLong(dyncall, static_cast<DClonglong>(r.i64));
							break;
						case nyt_u32:
							dcArgInt(dyncall, static_cast<DCint>(r.u64));
							break;
						case nyt_i32:
							dcArgInt(dyncall, static_cast<DCint>(r.i64));
							break;
						case nyt_ptr:
							dcArgPointer(dyncall, reinterpret_cast<DCpointer>(r.u64));
							break;
						case nyt_u16:
							dcArgShort(dyncall, static_cast<DCshort>(r.u64));
							break;
						case nyt_i16:
							dcArgShort(dyncall, static_cast<DCshort>(r.i64));
							break;
						case nyt_u8:
							dcArgChar(dyncall, static_cast<DCchar>(r.u64));
							break;
						case nyt_i8:
							dcArgChar(dyncall, static_cast<DCchar>(r.i64));
							break;
						case nyt_f32:
							dcArgFloat(dyncall, static_cast<DCfloat>(r.f64));
							break;
						case nyt_f64:
							dcArgDouble(dyncall, static_cast<DCdouble>(r.f64));
							break;
						case nyt_bool:
							dcArgBool(dyncall, static_cast<DCbool>(r.u64));
							break;
						case nyt_void:
						case nyt_any:
							emitInvalidIntrinsicParamType();
					}
				}
				funcparamCount = 0;

				switch (intrinsic.rettype)
				{
					case nyt_u64:
						registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallLongLong(dyncall, intrinsic.callback));
						break;
					case nyt_i64:
						registers[opr.lvid].i64 = static_cast<int64_t>(dcCallLongLong(dyncall, intrinsic.callback));
						break;
					case nyt_u32:
						registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallInt(dyncall, intrinsic.callback));
						break;
					case nyt_i32:
						registers[opr.lvid].i64 = static_cast<int64_t>(dcCallInt(dyncall, intrinsic.callback));
						break;
					case nyt_ptr:
						registers[opr.lvid].u64 = reinterpret_cast<uint64_t>(dcCallPointer(dyncall, intrinsic.callback));
						break;
					case nyt_u16:
						registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallShort(dyncall, intrinsic.callback));
						break;
					case nyt_i16:
						registers[opr.lvid].i64 = static_cast<int64_t>(dcCallShort(dyncall, intrinsic.callback));
						break;
					case nyt_u8:
						registers[opr.lvid].u64 = static_cast<uint64_t>(dcCallChar(dyncall, intrinsic.callback));
						break;
					case nyt_i8:
						registers[opr.lvid].i64 = static_cast<int64_t>(dcCallChar(dyncall, intrinsic.callback));
						break;
					case nyt_f32:
						registers[opr.lvid].f64 = static_cast<float>(dcCallFloat(dyncall, intrinsic.callback));
						break;
					case nyt_f64:
						registers[opr.lvid].f64 = static_cast<double>(dcCallDouble(dyncall, intrinsic.callback));
						break;
					case nyt_bool:
						registers[opr.lvid].u64 = (dcCallBool(dyncall, intrinsic.callback) ? 1 : 0);
						break;
					case nyt_void:
						dcCallVoid(dyncall, intrinsic.callback);
						break;
					case nyt_any:
						emitInvalidReturnType();
				}
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fadd>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].f64 = registers[opr.lhs].f64 + registers[opr.rhs].f64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fsub>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].f64 = registers[opr.lhs].f64 - registers[opr.rhs].f64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fmul>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].f64 = registers[opr.lhs].f64 * registers[opr.rhs].f64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fdiv>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				auto r = registers[opr.rhs].f64;
				if (YUNI_UNLIKELY((uint64_t)r == 0))
					emitDividedByZero();
				registers[opr.lvid].f64 = registers[opr.lhs].f64 / r;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::add>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 + registers[opr.rhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::sub>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 - registers[opr.rhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::mul>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 * registers[opr.rhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::div>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				auto r = registers[opr.rhs].u64;
				if (YUNI_UNLIKELY(r == 0))
					emitDividedByZero();
				registers[opr.lvid].u64 = registers[opr.lhs].u64 / r;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::imul>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = static_cast<uint64_t>(
																static_cast<int64_t>(registers[opr.lhs].u64) * static_cast<int64_t>(registers[opr.rhs].u64) );
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::idiv>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				auto r = static_cast<int64_t>(registers[opr.rhs].u64);
				if (YUNI_UNLIKELY(r == 0))
					emitDividedByZero();
				registers[opr.lvid].u64 = static_cast<uint64_t>(static_cast<int64_t>(registers[opr.lhs].u64) / r);
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::eq>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].u64 == registers[opr.rhs].u64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::neq>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].u64 != registers[opr.rhs].u64) ? 1 : 0;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::lt>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].u64 < registers[opr.rhs].u64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::lte>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].u64 <= registers[opr.rhs].u64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::ilt>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].i64 < registers[opr.rhs].i64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::ilte>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].i64 <= registers[opr.rhs].i64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::gt>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].u64 > registers[opr.rhs].u64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::gte>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].u64 >= registers[opr.rhs].u64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::igt>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].i64 > registers[opr.rhs].i64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::igte>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].i64 >= registers[opr.rhs].i64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::flt>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].f64 < registers[opr.rhs].f64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::flte>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].f64 <= registers[opr.rhs].f64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fgt>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].f64 > registers[opr.rhs].f64) ? 1 : 0;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fgte>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = (registers[opr.lhs].f64 >= registers[opr.rhs].f64) ? 1 : 0;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::opand>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 & registers[opr.rhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::opor>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 | registers[opr.rhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::opxor>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 ^ registers[opr.rhs].u64;
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::opmod>& opr)
			{
				VM_PRINT_OPCODE(operands);
				assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
				registers[opr.lvid].u64 = registers[opr.lhs].u64 % registers[opr.rhs].u64;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::push>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				funcparams[funcparamCount++].u64 = registers[operands.lvid].u64;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::ret>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				retRegister = registers[operands.lvid].u64;
				sequence.get().invalidateCursor(*cursor);
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::store>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid   < registerCount);
				assert(operands.source < registerCount);
				registers[operands.lvid] = registers[operands.source];
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::storeText>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid   < registerCount);
				registers[operands.lvid].u64 =
					reinterpret_cast<uint64_t>(sequence.get().stringrefs[operands.text].c_str());
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				registers[operands.lvid].u64 = operands.value.u64;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::classdefsizeof>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				assert(map.findAtom(operands.type) != nullptr);
				registers[operands.lvid].u64 = map.findAtom(operands.type)->runtimeSizeof();
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
			{
				assert(operands.lvid < registerCount);
				VM_PRINT_OPCODE(operands);
				call(operands.lvid, operands.ptr2func, operands.instanceid);
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fieldset>& operands)
			{
				assert(operands.self < registerCount);
				assert(operands.lvid < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.self].u64);
				VM_CHECK_POINTER(object, operands);
				object[1 + operands.var] = registers[operands.lvid].u64;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::fieldget>& operands)
			{
				assert(operands.self < registerCount);
				assert(operands.lvid < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.self].u64);
				VM_CHECK_POINTER(object, operands);
				registers[operands.lvid].u64 = object[1 + operands.var];
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::label>& operands)
			{
				if (operands.label > upperLabelID)
					upperLabelID = operands.label;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::jmp>& operands)
			{
				gotoLabel(operands.label);
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::jnz>& operands)
			{
				if (registers[operands.lvid].u64 != 0)
				{
					registers[operands.result].u64 = 1;
					gotoLabel(operands.label);
				}
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::jz>& operands)
			{
				if (registers[operands.lvid].u64 == 0)
				{
					registers[operands.result].u64 = 0;
					gotoLabel(operands.label);
				}
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::ref>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
				VM_CHECK_POINTER(object, operands);
				++(object[0]);
			}

			inline void visit(const IR::ISA::Operand<IR::ISA::Op::unref>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
				VM_CHECK_POINTER(object, operands);
				if (0 == --(object[0]))
					destroy(object, operands.atomid, operands.instanceid);
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::dispose>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
				VM_CHECK_POINTER(object, operands);
				destroy(object, operands.atomid, operands.instanceid);
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				(void) operands;
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::memalloc>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				assert(operands.regsize < registerCount);

				size_t size;
				if (sizeof(size_t) < sizeof(uint64_t))
				{
					// size_t is less than a u64 (32bits platform probably)
					// so it is possible to ask more than the system can provide
					auto request = registers[operands.regsize].u64;
					if (std::numeric_limits<size_t>::max() <= request)
						return emitBadAlloc();
					size = static_cast<size_t>(request);
				}
				else
				{
					// 64bits platform
					size = static_cast<size_t>(registers[operands.regsize].u64);
				}

				size += extraObjectSize();

				uint64_t* pointer = allocateraw<uint64_t>(size);
				if (YUNI_UNLIKELY(!pointer))
					return emitBadAlloc();
				if (debugmode)
					memset(pointer, patternAlloc, size);

				pointer[0] = 0; // init ref counter
				registers[operands.lvid].u64 = reinterpret_cast<uint64_t>(pointer);

				memchecker.hold(pointer, size, operands.lvid);
			}


			inline void visit(const IR::ISA::Operand<IR::ISA::Op::memrealloc>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				assert(operands.oldsize < registerCount);
				assert(operands.newsize < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);

				size_t oldsize = static_cast<size_t>(registers[operands.oldsize].u64);
				size_t newsize = static_cast<size_t>(registers[operands.newsize].u64);
				oldsize += extraObjectSize();
				newsize += extraObjectSize();

				if (object)
				{
					// checking the input pointer
					VM_CHECK_POINTER(object, operands);
					if (YUNI_UNLIKELY(not memchecker.checkObjectSize(object, static_cast<size_t>(oldsize))))
						return emitPointerSizeMismatch(object, oldsize);

					memchecker.forget(object);
				}

				auto* newptr = (uint64_t*) allocator.reallocate(&allocator, object, oldsize, newsize);
				registers[operands.lvid].u64 = reinterpret_cast<uint64_t>(newptr);

				if (newptr)
				{
					// the nointer has been successfully reallocated - keeping a reference
					memchecker.hold(newptr, newsize, operands.lvid);
				}
				else
				{
					// `realloc` may not release the input pointer if the
					// allocation failed
					deallocate(object, oldsize);
					emitBadAlloc();
				}
			}


			void visit(const IR::ISA::Operand<IR::ISA::Op::memfree>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				assert(operands.regsize < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
				if (object)
				{
					VM_CHECK_POINTER(object, operands);
					size_t size = static_cast<size_t>(registers[operands.regsize].u64);
					size += extraObjectSize();

					if (YUNI_UNLIKELY(not memchecker.checkObjectSize(object, static_cast<size_t>(size))))
						return emitPointerSizeMismatch(object, size);

					if (debugmode)
						memset(object, patternFree, size);

					deallocate(object, size);
					memchecker.forget(object);
				}
			}

			void visit(const IR::ISA::Operand<IR::ISA::Op::memfill>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);
				assert(operands.regsize < registerCount);
				assert(operands.pattern < registerCount);

				uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
				VM_CHECK_POINTER(object, operands);
				size_t size = static_cast<size_t>(registers[operands.regsize].u64);
				size += extraObjectSize();
				uint8_t pattern = static_cast<uint8_t>(registers[operands.pattern].u64);

				if (YUNI_UNLIKELY(not memchecker.checkObjectSize(object, static_cast<size_t>(size))))
					return emitPointerSizeMismatch(object, size);

				memset(object, pattern, size);
			}


			void visit(const IR::ISA::Operand<IR::ISA::Op::opassert>& operands)
			{
				VM_PRINT_OPCODE(operands);
				assert(operands.lvid < registerCount);

				if (YUNI_UNLIKELY(registers[operands.lvid].u64 == 0))
					return emitAssert();
			}

			template<IR::ISA::Op O> void visit(const IR::ISA::Operand<O>& operands)
			{
				VM_PRINT_OPCODE(operands); // FALLBACK
				(void) operands; // unused
				return emitUnexpectedOpcode(IR::ISA::Operand<O>::opname());
			}


			uint64_t invoke(const IR::Sequence& callee)
			{
				const uint32_t framesize = callee.at<IR::ISA::Op::stacksize>(0).add;
				#ifndef NDEBUG
				assert(framesize < 1024 * 1024);
				registerCount = framesize;
				assert(callee.at<IR::ISA::Op::stacksize>(0).opcode == (uint32_t) IR::ISA::Op::stacksize);
				#endif

				registers = stack.push(framesize);
				registers[0].u64 = 0;
				sequence = std::cref(callee);
				upperLabelID = 0;

				// retrieve parameters for the func
				for (uint32_t i = 0; i != funcparamCount; ++i)
					registers[i + 2].u64 = funcparams[i].u64; // 2-based
				funcparamCount = 0;

				//
				// iterate through all opcodes
				//
				callee.each(*this, 1); // offset: 1, avoid blueprint pragma

				stack.pop(framesize);
				return retRegister;
			}


			inline void call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid)
			{
				assert(retlvid < registerCount);
				// save the current stack frame
				auto* storestackptr = registers;
				auto storesequence = sequence;
				auto* storecursor = cursor;
				#ifndef NDEBUG
				auto  storestckfrmsize = registerCount;
				#endif
				auto labelid = upperLabelID;

				uint32_t memcheckPreviousAtomid = memchecker.atomid();
				stacktrace.push(atomfunc, instanceid);

				// call
				uint64_t ret = invoke(map.sequence(atomfunc, instanceid));

				// restore the previous stack frame and store the result of the call
				upperLabelID = labelid;
				registers = storestackptr;
				registers[retlvid].u64 = ret;
				sequence = storesequence;
				cursor = storecursor;
				#ifndef NDEBUG
				registerCount = storestckfrmsize;
				#endif
				stacktrace.pop();
				memchecker.atomid(memcheckPreviousAtomid);
			}

		}; // struct Executor


	} // anonymous namespace





	bool ThreadContext::invoke(uint64_t& exitstatus, const IR::Sequence& callee, uint32_t atomid, uint32_t instanceid)
	{
		// if something happens
		exitstatus = static_cast<uint64_t>(-1);
		// result
		bool success = false;

		if (!cf.on_thread_create
			or nyfalse != cf.on_thread_create(program.self(), self(), nullptr, name.c_str(), name.size()))
		{
			Executor executor{*this, callee};
			executor.stacktrace.push(atomid, instanceid);
			if (setjmp(executor.jump_buffer) != 666)
			{
				executor.invoke(callee);
				exitstatus = executor.retRegister;
				success = true;
			}
			else
			{
				// execution of the program failed
			}

			if (cf.on_thread_destroy)
				cf.on_thread_destroy(program.self(), self());
		}
		return success;
	}





} // namespace VM
} // namespace Nany
