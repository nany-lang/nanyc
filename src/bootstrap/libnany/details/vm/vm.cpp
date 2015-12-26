#include "vm.h"
#include "details/ir/isa/data.h"
#include "details/atom/atom.h"
#include "backtrace.h"
#include <iostream>

using namespace Yuni;

//! Memory leaks
#ifndef NDEBUG
#define NANY_VM_WITH_MEMORY_LEAKS_DETECTION 1
#else
#define NANY_VM_WITH_MEMORY_LEAKS_DETECTION 0
#endif

//! Print opcodes executed by the vm
#define NANY_VM_PRINT_OPCODES 0

//! Keep stack traces
#define NANY_VM_STACK_TRACES 1





#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
#define VM_CHECK_POINTER(P,LVID) do { if (!(P) or ownedPointers.count(P) == 0) { \
	/*assert(false and "invalid pointer");*/ \
	throw (String{"invalid pointer "} << (P) << " not found, opc: " << (Nany::IR::ISA::print(program.get(), operands, &map)) \
		<< '\n' << dumpBacktrace()); \
	} } while (0)
#else
#define VM_CHECK_POINTER(P,LVID)
#endif


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

		class Engine final
		{
		public:
			Engine(const AtomMap& map, nycontext_t&, const IR::Program& program);

			uint64_t call(const IR::Program& program);

			void checkForMemoryLeaks() const;

			String dumpBacktrace() const;

		public:
			//! Registers for the current stack frame
			uint64_t* registers = nullptr;
			uint64_t  retRegister = 0;
			//! Number of pushed parameters
			uint32_t funcparamCount = 0; // parameters are 2-based
			//! all pushed parameters
			uint64_t funcparams[Config::maxPushedParameters];

			nycontext_t& context;
			std::reference_wrapper<const IR::Program> program;
			const AtomMap& map;

			#ifndef NDEBUG
			uint32_t stackframeSize = 0;
			uint32_t stackframe = 0;
			#endif

			#if NANY_VM_STACK_TRACES != 0
			Stack::List stack;
			#endif

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
			#ifndef NDEBUG // accept those opcode for debugging purposes
			void visit(const IR::ISA::Operand<IR::ISA::Op::comment>&) {}
			void visit(const IR::ISA::Operand<IR::ISA::Op::scope>&) {}
			void visit(const IR::ISA::Operand<IR::ISA::Op::end>&) {}
			#endif
			template<enum IR::ISA::Op O> void visit(const IR::ISA::Operand<O>& operands);

			void destroy(uint64_t* object, uint32_t dtorid, uint32_t instanceid);
			void call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid);

		private:
			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			std::unordered_map<uint64_t*,uint64_t> ownedPointers;
			#endif
		}; // class Engine




		inline Engine::Engine(const AtomMap& map, nycontext_t& context, const IR::Program& program)
			: context(context)
			, program(std::cref(program))
			, map(map)
		{}


		inline String Engine::dumpBacktrace() const
		{
			#if NANY_VM_STACK_TRACES != 0
			return stack.dump(map);
			#else
			return String{};
			#endif
		}


		inline void Engine::checkForMemoryLeaks() const
		{
			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			if (unlikely(not ownedPointers.empty()))
			{
				String msg;
				msg.reserve(64 + (uint32_t) ownedPointers.size() * 36); // arbitrary

				msg << "\n\n=== nany vm: memory leaks detected in ";
				msg << ownedPointers.size() << " blocks ===\n";
				std::unordered_map<uint64_t, uint64_t> loses;
				for (auto& pair: ownedPointers)
					loses[pair.second]++;

				for (auto& pair: loses)
				{
					msg << "    block " << pair.first << " bytes * " << pair.second;
					msg << " = ";
					msg << (pair.second * pair.first);
					msg << " bytes\n";
				}

				msg << '\n';
				context.console.write_stderr(&context, msg.c_str(), msg.size());
			}
			#endif
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

			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			ownedPointers.erase(object);
			#endif
		}


		inline void Engine::call(uint32_t retlvid, uint32_t atomfunc, uint32_t instanceid)
		{
			assert(retlvid < stackframeSize);
			// save the current stack frame
			auto* storestackptr = registers;
			auto storeprogram = program;
			auto* storecursor = cursor;
			#ifndef NDEBUG
			auto  storestckfrmsize = stackframeSize;
			#endif

			#if NANY_VM_STACK_TRACES != 0
			stack.push(atomfunc, instanceid);
			#endif

			// call
			uint64_t ret = call(map.program(atomfunc, instanceid));

			// restore the previous stack frame and store the result of the call
			registers = storestackptr;
			registers[retlvid] = ret;
			program = storeprogram;
			cursor = storecursor;
			#ifndef NDEBUG
			stackframeSize = storestckfrmsize;
			#endif
			#if NANY_VM_STACK_TRACES != 0
			stack.pop();
			#endif
		}



		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::ref>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid]);
			VM_CHECK_POINTER(object, operands);
			++(object[0]);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::unref>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid]);
			VM_CHECK_POINTER(object, operands);
			if (0 == --(object[0]))
				destroy(object, operands.atomid, operands.instanceid);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::dispose>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid]);
			VM_CHECK_POINTER(object, operands);
			destroy(object, operands.atomid, operands.instanceid);
		}



		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			(void) operands;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::memalloc>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			assert(operands.regsize < stackframeSize);

			uint64 size = registers[operands.regsize];
			size += sizeof(uint64_t); // reference counter

			uint64_t* pointer = (uint64_t*) context.memory.allocate(&context, size);
			if (unlikely(!pointer))
				throw std::bad_alloc();

			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			memset(pointer, 0xEF, size);
			#endif

			pointer[0] = 0; // init ref counter
			registers[operands.lvid] = reinterpret_cast<uint64_t>(pointer);

			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			ownedPointers.insert(std::make_pair(pointer, size));
			#endif
		}


		void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::memfree>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			assert(operands.regsize < stackframeSize);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.lvid]);
			VM_CHECK_POINTER(object, operands);
			uint64 size = registers[operands.regsize];
			size += sizeof(uint64_t); // reference counter

			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			if (size != ownedPointers[object])
				throw (String{"pointer size mismatch: got "} << size << ", expected " << ownedPointers[object]);
			memset(object, 0xCD, size);
			#endif

			context.memory.release(&context, object, size);

			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			ownedPointers.erase(object);
			#endif
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::push>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			funcparams[funcparamCount++] = registers[operands.lvid];
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::ret>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			retRegister = registers[operands.lvid];
			program.get().invalidateCursor(*cursor);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::store>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid   < stackframeSize);
			assert(operands.source < stackframeSize);
			registers[operands.lvid] = registers[operands.source];
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::storeText>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid   < stackframeSize);
			registers[operands.lvid] = reinterpret_cast<uint64_t>(program.get().stringrefs[operands.text].c_str());
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			registers[operands.lvid] = operands.value.u64;
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::classdefsizeof>& operands)
		{
			VM_PRINT_OPCODE(operands);
			assert(operands.lvid < stackframeSize);
			assert(map.findAtom(operands.type) != nullptr);
			registers[operands.lvid] = map.findAtom(operands.type)->runtimeSizeof();
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
		{
			assert(operands.lvid < stackframeSize);
			VM_PRINT_OPCODE(operands);
			call(operands.lvid, operands.ptr2func, operands.instanceid);
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fieldset>& operands)
		{
			assert(operands.self < stackframeSize);
			assert(operands.lvid < stackframeSize);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.self]);
			VM_CHECK_POINTER(object, operands);
			object[1 + operands.var] = registers[operands.lvid];
		}


		inline void Engine::visit(const IR::ISA::Operand<IR::ISA::Op::fieldget>& operands)
		{
			assert(operands.self < stackframeSize);
			assert(operands.lvid < stackframeSize);

			uint64_t* object = reinterpret_cast<uint64_t*>(registers[operands.self]);
			VM_CHECK_POINTER(object, operands);
			registers[operands.lvid] = object[1 + operands.var];
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
			const uint32_t framesize = callee.at<IR::ISA::Op::stacksize>(0).add + 1; // 1-based
			#ifndef NDEBUG
			++stackframe;
			assert(framesize < 1024 * 1024);
			stackframeSize = framesize;
			assert(callee.at<IR::ISA::Op::stacksize>(0).opcode == (uint32_t) IR::ISA::Op::stacksize);
			#endif

			uint64_t stackvalues[framesize];
			#if NANY_VM_WITH_MEMORY_LEAKS_DETECTION != 0
			memset(stackvalues, 0xDE, sizeof(stackvalues));
			#endif
			stackvalues[0] = 0;
			registers = stackvalues;
			program = std::cref(callee);

			// retrieve parameters for the func
			for (uint32_t i = 0; i != funcparamCount; ++i)
				stackvalues[i + 2] = funcparams[i]; // 2-based
			funcparamCount = 0;

			callee.each(*this, 1); // offset: 1, avoid blueprint pragma

			#ifndef NDEBUG
			--stackframe;
			#endif
			return retRegister;
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

			engine.checkForMemoryLeaks();
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
			String txt; txt << "error: ICE: unexpected error\n";
			context.console.write_stderr(&context, txt.c_str(), txt.size());
		}

		// always flush to notify any listener in embeded mode
		context.console.flush_stderr(&context);
		context.console.flush_stdout(&context);
		return static_cast<int>(returnvalue);
	}




} // namespace VM
} // namespace Nany
