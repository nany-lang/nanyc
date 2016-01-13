#include "vm.h"
#include "details/ir/isa/data.h"
#include "details/atom/atom.h"
#include "stacktrace.h"
#include "memchecker.h"
#include <iostream>

using namespace Yuni;

//! Print opcodes executed by the vm
#define NANY_VM_PRINT_OPCODES 0




#define VM_CHECK_POINTER(P,LVID) do { if (YUNI_UNLIKELY(not memchecker.has((P)))) { \
	/*assert(false and "invalid pointer");*/ \
	throw (String{"invalid pointer "} << (P) << " not found, opc: " << (Nany::IR::ISA::print(program.get(), operands, &map)) \
		<< '\n' << stacktrace.dump(map)); \
	} } while (0)



#if NANY_VM_PRINT_OPCODES != 0
#define VM_PRINT_OPCODE(O)  do { std::cout << "== nany:vm ==  " \
	<< Nany::IR::ISA::print(program.get(), operands, &map) << '\n';} while (0)
#else
#define VM_PRINT_OPCODE(O)
#endif





namespace Nany
{
namespace VM
{


	namespace // anonymous
	{

		union DataRegister
		{
			uint64_t u64;
			int64_t i64;
			double f64;
		};



		class Engine final
		{
		public:
			Engine(const AtomMap& map, nycontext_t&, const IR::Program& program);
			~Engine();

			uint64_t call(const IR::Program& program);


		public:
			//! Registers for the current stack frame
			DataRegister* registers = nullptr;
			uint64_t  retRegister = 0;
			//! Number of pushed parameters
			uint32_t funcparamCount = 0; // parameters are 2-based
			//! all pushed parameters
			uint64_t funcparams[Config::maxPushedParameters];

			//! Stack trace
			Stacktrace<true> stacktrace;

			#ifndef NDEBUG
			//! Total number of registers in the current frame
			uint32_t registerCount = 0;
			#endif

			//! upper label id encountered so far
			uint32_t upperLabelID = 0;
			//! Atom collection references
			const AtomMap& map;

			//! Source program
			std::reference_wrapper<const IR::Program> program;
			//! User context
			nycontext_t& context;

			//! Reference to the current iterator
			const IR::Instruction** cursor = nullptr;


		private:
			friend class IR::Program;
			void visit(const IR::ISA::Operand<IR::ISA::Op::stackalloc>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::call>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::classdefsizeof>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fieldget>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fieldset>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::memalloc>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::memfree>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::ref>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::unref>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::dispose>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::push>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::ret>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::store>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::storeText>&);

			void visit(const IR::ISA::Operand<IR::ISA::Op::label>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::jmp>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::jnz>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::jz>&);

			void visit(const IR::ISA::Operand<IR::ISA::Op::add>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::sub>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::mul>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::imul>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::div>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::idiv>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fadd>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fsub>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fmul>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fdiv>&);

			void visit(const IR::ISA::Operand<IR::ISA::Op::opand>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::opor>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::opxor>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::opmod>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::lt>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::lte>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::ilt>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::ilte>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::gt>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::gte>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::igt>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::igte>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::flt>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::flte>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fgt>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::fgte>&);

			void visit(const IR::ISA::Operand<IR::ISA::Op::eq>&);
			void visit(const IR::ISA::Operand<IR::ISA::Op::neq>&);

			// accept those opcode for debugging purposes
			void visit(const IR::ISA::Operand<IR::ISA::Op::comment>&) {}
			void visit(const IR::ISA::Operand<IR::ISA::Op::scope>&) {}
			void visit(const IR::ISA::Operand<IR::ISA::Op::end>&) {}
			void visit(const IR::ISA::Operand<IR::ISA::Op::nop>&) {}
			template<enum IR::ISA::Op O> void visit(const IR::ISA::Operand<O>& operands);


			void destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid);
			void call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid);
			void gotoLabel(uint32_t label);

		private:
			MemChecker<true> memchecker;

		}; // class Engine




		inline Engine::Engine(const AtomMap& map, nycontext_t& context, const IR::Program& program)
			: map(map)
			, program(std::cref(program))
			, context(context)
		{}


		inline Engine::~Engine()
		{
			memchecker.printLeaksIfAny(context);
		}


		void Engine::destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid)
		{
			// the dtor function to call
			const Atom* dtor = map.findAtom(dtorid);
			assert(dtor != nullptr);

			// the parent class
			const Atom* classobject = dtor->parent;
			assert(classobject != nullptr);

			// its size
			uint64_t classsizeof = classobject->runtimeSizeof();
			classsizeof += sizeof(uint64_t); // internal ref counting

			if (instanceid != (uint32_t) -1)
			{
				// reset parameters for func call
				funcparamCount = 1;
				funcparams[0] = reinterpret_cast<uint64_t>(object); // self
				// func call
				call(0, dtor->atomid, instanceid);
			}

			// sandbox release
			context.memory.release(&context, object, classsizeof);
			memchecker.forget(object);
		}


		inline void Engine::call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid)
		{
			assert(retlvid < registerCount);
			// save the current stack frame
			auto* storestackptr = registers;
			auto storeprogram = program;
			auto* storecursor = cursor;
			#ifndef NDEBUG
			auto  storestckfrmsize = registerCount;
			#endif

			uint32_t memcheckPreviousAtomid = memchecker.atomid();
			stacktrace.push(atomfunc, instanceid);

			// call
			uint64_t ret = call(map.program(atomfunc, instanceid));

			// restore the previous stack frame and store the result of the call
			registers = storestackptr;
			registers[retlvid].u64 = ret;
			program = storeprogram;
			cursor = storecursor;
			#ifndef NDEBUG
			registerCount = storestckfrmsize;
			#endif
			stacktrace.pop();
			memchecker.atomid(memcheckPreviousAtomid);
		}


		inline void Engine::gotoLabel(uint32_t label)
		{
			if (label > upperLabelID)
				program.get().jumpToLabelForward(*cursor, label);
			else
				program.get().jumpToLabelBackward(*cursor, label);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fadd>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].f64 = registers[opr.lhs].f64 + registers[opr.rhs].f64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fsub>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].f64 = registers[opr.lhs].f64 - registers[opr.rhs].f64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fmul>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].f64 = registers[opr.lhs].f64 * registers[opr.rhs].f64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fdiv>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			auto r = registers[opr.rhs].f64;
			if (YUNI_UNLIKELY((uint64_t)r == 0))
				throw std::overflow_error("Divide by zero exception");
			registers[opr.lvid].f64 = registers[opr.lhs].f64 / r;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::add>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 + registers[opr.rhs].u64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::sub>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 - registers[opr.rhs].u64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::mul>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 * registers[opr.rhs].u64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::div>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			auto r = registers[opr.rhs].u64;
			if (YUNI_UNLIKELY(r == 0))
				throw std::overflow_error("Divide by zero exception");
			registers[opr.lvid].u64 = registers[opr.lhs].u64 / r;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::imul>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = static_cast<uint64_t>(
				static_cast<int64_t>(registers[opr.lhs].u64) * static_cast<int64_t>(registers[opr.rhs].u64) );
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::idiv>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			auto r = static_cast<int64_t>(registers[opr.rhs].u64);
			if (YUNI_UNLIKELY(r == 0))
				throw std::overflow_error("Divide by zero exception");
			registers[opr.lvid].u64 = static_cast<uint64_t>(static_cast<int64_t>(registers[opr.lhs].u64) / r);
		}



		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::eq>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].u64 == registers[opr.rhs].u64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::neq>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].u64 != registers[opr.rhs].u64) ? 1 : 0;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::lt>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].u64 < registers[opr.rhs].u64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::lte>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].u64 <= registers[opr.rhs].u64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::ilt>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].i64 < registers[opr.rhs].i64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::ilte>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].i64 <= registers[opr.rhs].i64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::gt>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].u64 > registers[opr.rhs].u64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::gte>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].u64 >= registers[opr.rhs].u64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::igt>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].i64 > registers[opr.rhs].i64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::igte>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].i64 >= registers[opr.rhs].i64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::flt>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].f64 < registers[opr.rhs].f64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::flte>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].f64 <= registers[opr.rhs].f64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fgt>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].f64 > registers[opr.rhs].f64) ? 1 : 0;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fgte>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = (registers[opr.lhs].f64 >= registers[opr.rhs].f64) ? 1 : 0;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::opand>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 & registers[opr.rhs].u64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::opor>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 | registers[opr.rhs].u64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::opxor>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 ^ registers[opr.rhs].u64;
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::opmod>& opr)
		{
			VM_PRINT_OPCODE(operands);
			assert(opr.lvid < registerCount and opr.lhs < registerCount and opr.rhs < registerCount);
			registers[opr.lvid].u64 = registers[opr.lhs].u64 % registers[opr.rhs].u64;
		}



		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::ref>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
			VM_CHECK_POINTER(object, operands);
			++(object[0]);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::unref>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
			VM_CHECK_POINTER(object, operands);
			if (0 == --(object[0]))
				destroy(object, operands.atomid, operands.instanceid);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::dispose>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
			VM_CHECK_POINTER(object, operands);
			destroy(object, operands.atomid, operands.instanceid);
		}



		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			(void) operands;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::memalloc>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			assert(operands.regsize < registerCount);

			auto size = registers[operands.regsize].u64;
			size += sizeof(uint64_t); // reference counter

			uint64_t* pointer = (uint64_t*) context.memory.allocate(&context, size);
			if (unlikely(!pointer))
				throw std::bad_alloc();

			#ifndef NDEBUG
			memset(pointer, 0xEF, size);
			#endif

			pointer[0] = 0; // init ref counter
			registers[operands.lvid].u64 = reinterpret_cast<uint64_t>(pointer);

			memchecker.hold(pointer, static_cast<size_t>(size), operands.lvid);
		}


		void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::memfree>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			assert(operands.regsize < registerCount);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid].u64);
			VM_CHECK_POINTER(object, operands);
			uint64 size = registers[operands.regsize].u64;
			size += sizeof(uint64_t); // reference counter

			if (YUNI_UNLIKELY(not memchecker.checkObjectSize(object, static_cast<size_t>(size))))
				throw (String{"pointer "} << (void*) object << " size mismatch: got " << size);

			#ifndef NDEBUG
			memset(object, 0xCD, size);
			#endif

			context.memory.release(&context, object, size);
			memchecker.forget(object);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::push>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			funcparams[funcparamCount++] = registers[operands.lvid].u64;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::ret>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			retRegister = registers[operands.lvid].u64;
			program.get().invalidateCursor(*cursor);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::store>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid   < registerCount);
			assert(operands.source < registerCount);
			registers[operands.lvid] = registers[operands.source];
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::storeText>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid   < registerCount);
			registers[operands.lvid].u64 =
				reinterpret_cast<uint64_t>(program.get().stringrefs[operands.text].c_str());
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			registers[operands.lvid].u64 = operands.value.u64;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::classdefsizeof>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			assert(map.findAtom(operands.type) != nullptr);
			registers[operands.lvid].u64 = map.findAtom(operands.type)->runtimeSizeof();
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
		{
			assert(operands.lvid < registerCount);
			VM_PRINT_OPCODE(operands);
			call(operands.lvid, operands.ptr2func, operands.instanceid);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fieldset>& operands)
		{
			assert(operands.self < registerCount);
			assert(operands.lvid < registerCount);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.self].u64);
			VM_CHECK_POINTER(object, operands);
			object[1 + operands.var] = registers[operands.lvid].u64;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fieldget>& operands)
		{
			assert(operands.self < registerCount);
			assert(operands.lvid < registerCount);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.self].u64);
			VM_CHECK_POINTER(object, operands);
			registers[operands.lvid].u64 = object[1 + operands.var];
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::label>& operands)
		{
			if (operands.label > upperLabelID)
				upperLabelID = operands.label;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::jmp>& operands)
		{
			gotoLabel(operands.label);
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::jnz>& operands)
		{
			if (registers[operands.lvid].u64 != 0)
			{
				registers[operands.result].u64 = 1;
				gotoLabel(operands.label);
			}
		}

		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::jz>& operands)
		{
			if (registers[operands.lvid].u64 == 0)
			{
				registers[operands.result].u64 = 0;
				gotoLabel(operands.label);
			}
		}


		template<enum IR::ISA::Op O> void Engine::visit(const IR::ISA::Operand<O>& operands)
		{
			VM_PRINT_OPCODE(operands);
			(void) operands; // unused
			throw String{}
				<< "error: unexpected opcode '" << IR::ISA::Operand<O>::opname() << "'\n";
		}


		uint64_t Engine::call(const IR::Program& callee)
		{
			try
			{
				const uint32_t framesize = callee.at<IR::ISA::Op::stacksize>(0).add + 1; // 1-based
				#ifndef NDEBUG
				assert(framesize < 1024 * 1024);
				registerCount = framesize;
				assert(callee.at<IR::ISA::Op::stacksize>(0).opcode == (uint32_t) IR::ISA::Op::stacksize);
				#endif

				DataRegister stackvalues[framesize];
				#ifndef NDEBUG
				memset(stackvalues, 0xDE, sizeof(stackvalues));
				#endif

				stackvalues[0].u64 = 0;
				registers = stackvalues;
				program = std::cref(callee);

				// retrieve parameters for the func
				for (uint32_t i = 0; i != funcparamCount; ++i)
					stackvalues[i + 2].u64 = funcparams[i]; // 2-based
				funcparamCount = 0;

				callee.each(*this, 1); // offset: 1, avoid blueprint pragma
				return retRegister;
			}
			catch (const std::bad_alloc&)
			{
				context.memory.on_not_enough_memory(&context);
			}
			catch (const std::exception& e)
			{
				String txt; txt << "exception: " << e.what() << '\n';
				context.console.write_stderr(&context, txt.c_str(), txt.size());
			}
			catch (const String& msg)
			{
				String txt; txt << "exception: " << msg << '\n';
				context.console.write_stderr(&context, txt.c_str(), txt.size());
			}
			catch (...)
			{
				String txt; txt << "exception: unexpected error\n";
				context.console.write_stderr(&context, txt.c_str(), txt.size());
			}

			const auto& txt = stacktrace.dump(map);
			context.console.write_stderr(&context, txt.c_str(), txt.size());
			return static_cast<uint64_t>(-1);
		}


	} // anonymous namespace







	int execute(bool& success, nycontext_t& context, const IR::Program& program, const AtomMap& map)
	{
		uint64_t returnvalue = 0;
		success = false;

		try
		{
			Engine engine{map, context, program};
			returnvalue = engine.call(program);
			success = true;
		}
		catch (const std::bad_alloc&)
		{
			context.memory.on_not_enough_memory(&context);
		}
		catch (const std::exception& e)
		{
			String txt; txt << "error: ICE: " << e.what() << '\n';
			context.console.write_stderr(&context, txt.c_str(), txt.size());
		}
		catch (const String& msg)
		{
			String txt; txt << "error: ICE: " << msg << '\n';
			context.console.write_stderr(&context, txt.c_str(), txt.size());
		}
		catch (...)
		{
			String txt;
			txt << "error: ICE: unexpected error\n";
			context.console.write_stderr(&context, txt.c_str(), txt.size());
		}

		// always flush to notify any listener in embeded mode
		context.console.flush_stderr(&context);
		context.console.flush_stdout(&context);
		return static_cast<int>(returnvalue);
	}




} // namespace VM
} // namespace Nany
