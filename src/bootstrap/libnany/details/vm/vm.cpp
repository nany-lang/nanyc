#include "vm.h"
#include "details/ir/isa/data.h"
#include "details/atom/atom.h"
#include "stacktrace.h"
#include "memchecker.h"
#include <iostream>
#include "dyncall/dyncall.h"

using namespace Yuni;

//! Print opcodes executed by the vm
#define NANY_VM_PRINT_OPCODES 0




#define VM_CHECK_POINTER(P,LVID) do { if (YUNI_UNLIKELY(not memchecker.has((P)))) { \
	/*assert(false and "invalid pointer");*/ \
	throw (String{"invalid pointer "} << (P) << " not found, opc: " \
		<< (Nany::IR::ISA::print(program.get(), operands, &map))); \
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


	struct CodeException: public std::exception
	{
		virtual const char* what() const throw() { return ""; }
	};




	struct ThreadContext final
	{
		//! Registers for the current stack frame
		DataRegister* registers = nullptr;
		//! Return value
		uint64_t  retRegister = 0;

		//! Number of pushed parameters
		uint32_t funcparamCount = 0; // parameters are 2-based
		//! all pushed parameters
		uint64_t funcparams[Config::maxPushedParameters];

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

		//! Source program
		std::reference_wrapper<const IR::Program> program;
		//! User context
		nycontext_t& context;

		DCCallVM* dyncall = nullptr;

		//! Reference to the current iterator
		const IR::Instruction** cursor = nullptr;


	public:
		ThreadContext(const AtomMap& map, nycontext_t& context, const IR::Program& program)
			: map(map)
			, program(std::cref(program))
			, context(context)
		{
			dyncall = dcNewCallVM(4096);
			dcMode(dyncall, DC_CALL_C_DEFAULT);
		}

		~ThreadContext()
		{
			if (dyncall)
				dcFree(dyncall);
			memchecker.printLeaksIfAny(context);
		}


		/*!
		** \brief Call the dtor of an object and free it
		*/
		void destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid)
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


		/*!
		** \brief Jump to / Execute a function
		*/
		void call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid)
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
			uint64_t ret = invoke(map.program(atomfunc, instanceid));

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


		/*!
		** \brief Jump to a label
		*/
		inline void gotoLabel(uint32_t label)
		{
			if (label > upperLabelID)
				program.get().jumpToLabelForward(*cursor, label);
			else
				program.get().jumpToLabelBackward(*cursor, label);
		}


		// accept those opcode for debugging purposes
		void visit(const IR::ISA::Operand<IR::ISA::Op::comment>&) {}
		void visit(const IR::ISA::Operand<IR::ISA::Op::scope>&) {}
		void visit(const IR::ISA::Operand<IR::ISA::Op::end>&) {}
		void visit(const IR::ISA::Operand<IR::ISA::Op::nop>&) {}


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
				throw std::overflow_error("Divide by zero exception");
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
				throw std::overflow_error("Divide by zero exception");
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
				throw std::overflow_error("Divide by zero exception");
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
			funcparams[funcparamCount++] = registers[operands.lvid].u64;
		}


		inline void visit(const IR::ISA::Operand<IR::ISA::Op::ret>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < registerCount);
			retRegister = registers[operands.lvid].u64;
			program.get().invalidateCursor(*cursor);
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
				reinterpret_cast<uint64_t>(program.get().stringrefs[operands.text].c_str());
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

			auto size = registers[operands.regsize].u64;
			size += sizeof(uint64_t); // reference counter

			uint64_t* pointer = (uint64_t*) context.memory.allocate(&context, size);
			if (unlikely(!pointer))
				throw std::bad_alloc();

			if (debugmode)
				memset(pointer, 0xEF, size);

			pointer[0] = 0; // init ref counter
			registers[operands.lvid].u64 = reinterpret_cast<uint64_t>(pointer);

			memchecker.hold(pointer, static_cast<size_t>(size), operands.lvid);
		}


		void visit(const IR::ISA::Operand<IR::ISA::Op::memfree>& operands)
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

			if (debugmode)
				memset(object, 0xCD, size);

			context.memory.release(&context, object, size);
			memchecker.forget(object);
		}


		template<enum IR::ISA::Op O> void visit(const IR::ISA::Operand<O>& operands)
		{
			VM_PRINT_OPCODE(operands); // FALLBACK
			(void) operands; // unused
			throw String{}
				<< "error: unexpected opcode '" << IR::ISA::Operand<O>::opname() << '\'';
		}


		uint64_t invoke(const IR::Program& callee)
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
				if (debugmode)
					memset(stackvalues, 0xDE, sizeof(stackvalues));

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
			catch (const CodeException&)
			{
				throw; // propagate error
			}
			catch (const std::bad_alloc&)
			{
				context.memory.on_not_enough_memory(&context);
			}
			catch (const std::exception& e)
			{
				String msg;
				msg << "\n\nerror: exception piko: " << e.what() << '\n';
				context.console.write_stderr(&context, msg.c_str(), msg.size());
			}
			catch (const String& incoming)
			{
				String msg;
				msg << "\n\nerror: exception: " << incoming << '\n';
				context.console.write_stderr(&context, msg.c_str(), msg.size());
			}
			catch (...)
			{
				AnyString msg{"\n\nerror: exception: unexpected error\n"};
				context.console.write_stderr(&context, msg.c_str(), msg.size());
			}

			stacktrace.dump(context, map);

			// invalid data related to allocated pointers
			memchecker.clear();
			throw CodeException{};
			return (uint64_t) -1;
		}



	}; // class ThreadContext



} // anonymous namespace







int execute(bool& success, nycontext_t& context, const IR::Program& program, const AtomMap& map)
{
	uint64_t returnvalue = 0;
	success = false;

	try
	{
		ThreadContext thrctx{map, context, program};
		returnvalue = thrctx.invoke(program);
		success = true;
	}
	catch (const CodeException&)
	{
		// error already handled
	}
	catch (const std::bad_alloc&)
	{
		context.memory.on_not_enough_memory(&context);
	}
	catch (const std::exception& e)
	{
		String msg; msg << "error: exception: " << e.what() << '\n';
		context.console.write_stderr(&context, msg.c_str(), msg.size());
	}
	catch (...)
	{
		AnyString txt{"error: exception received: aborting\n"};
		context.console.write_stderr(&context, txt.c_str(), txt.size());
	}

	// always flush to make sure that the listener will update the output
	// (mainly when embedded into a C/C++ application)
	context.console.flush_stderr(&context);
	context.console.flush_stdout(&context);
	return static_cast<int>(returnvalue);
}



} // namespace VM
} // namespace Nany
