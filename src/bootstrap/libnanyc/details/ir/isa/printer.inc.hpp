#pragma once
#include "opcodes.h"
#include "../sequence.h"
#include <stdio.h>
#include <stdlib.h>
#include "details/atom/atom-map.h"



namespace ny
{
namespace IR
{
namespace ISA
{
namespace // anonymous
{

	template<class S> struct Printer final
	{
		S& out;
		Yuni::String tabs;
		Yuni::String tmp;
		const Sequence& sequence;
		const Instruction** cursor = nullptr;
		const ny::AtomMap* atommap = nullptr;
		Yuni::ShortString64 lineHeader;
		uint32_t lastOffset = (uint32_t) -1;
		uint32_t offset = 0;


		Printer(S& out, const Sequence& sequence)
			: out(out), sequence(sequence)
		{
			lineHeader << "           ";
		}

		void indent()   { tabs.append("    ", 4); }
		void unindent() { tabs.chop(4); }
		void printEOL() { out += '\n'; }

		void printString(uint32_t sid)
		{
			auto text = sequence.stringrefs[sid];
			out << '@' << sid << "\"" << text << "\"";
		}

		S& line()
		{
			if (cursor)
			{
				auto offset = sequence.offsetOf(**cursor);
				if (offset != lastOffset)
				{
					lastOffset = offset;
					Yuni::ShortString64 offsetstr;
					offsetstr << ' ' << offset;
					Yuni::ShortString64 tmp;
					tmp.resize(8, ".");
					tmp.overwriteRight(offsetstr);
					out << tmp << " | ";
				}
				else
					out << lineHeader;
			}
			out << tabs;
			return out;
		}


		void print(const Operand<Op::nop>&)
		{
			line() << "nop";
		}


		void print(const Operand<Op::negation>& operands)
		{
			line() << '%' << operands.lvid << " = not %" << operands.lhs;
		}


		template<class T> void printOperator(const T& operands, const AnyString& opname)
		{
			line() << '%' << operands.lvid << " = %" << operands.lhs << ' ' << opname << " %" << operands.rhs;
		}

		void print(const Operand<Op::eq>& operands)
		{
			printOperator(operands, "eq");
		}

		void print(const Operand<Op::neq>& operands)
		{
			printOperator(operands, "neq");
		}

		void print(const Operand<Op::lt>& operands)
		{
			printOperator(operands, "lt");
		}

		void print(const Operand<Op::lte>& operands)
		{
			printOperator(operands, "lte");
		}

		void print(const Operand<Op::ilt>& operands)
		{
			printOperator(operands, "ilt");
		}

		void print(const Operand<Op::ilte>& operands)
		{
			printOperator(operands, "ilte");
		}

		void print(const Operand<Op::gt>& operands)
		{
			printOperator(operands, "gt");
		}

		void print(const Operand<Op::gte>& operands)
		{
			printOperator(operands, "gte");
		}

		void print(const Operand<Op::igt>& operands)
		{
			printOperator(operands, "igt");
		}

		void print(const Operand<Op::igte>& operands)
		{
			printOperator(operands, "igte");
		}

		void print(const Operand<Op::flt>& operands)
		{
			printOperator(operands, "flt");
		}

		void print(const Operand<Op::flte>& operands)
		{
			printOperator(operands, "flte");
		}

		void print(const Operand<Op::fgt>& operands)
		{
			printOperator(operands, "fgt");
		}

		void print(const Operand<Op::fgte>& operands)
		{
			printOperator(operands, "fgte");
		}


		void print(const Operand<Op::opand>& operands)
		{
			printOperator(operands, "and");
		}

		void print(const Operand<Op::opor>& operands)
		{
			printOperator(operands, "or");
		}

		void print(const Operand<Op::opxor>& operands)
		{
			printOperator(operands, "xor");
		}

		void print(const Operand<Op::opmod>& operands)
		{
			printOperator(operands, "mod");
		}

		void print(const Operand<Op::fadd>& operands)
		{
			printOperator(operands, "fadd");
		}

		void print(const Operand<Op::fsub>& operands)
		{
			printOperator(operands, "fsub");
		}

		void print(const Operand<Op::fmul>& operands)
		{
			printOperator(operands, "fmul");
		}

		void print(const Operand<Op::fdiv>& operands)
		{
			printOperator(operands, "fdiv");
		}

		void print(const Operand<Op::add>& operands)
		{
			printOperator(operands, "add");
		}

		void print(const Operand<Op::sub>& operands)
		{
			printOperator(operands, "sub");
		}

		void print(const Operand<Op::mul>& operands)
		{
			printOperator(operands, "mul");
		}

		void print(const Operand<Op::imul>& operands)
		{
			printOperator(operands, "imul");
		}

		void print(const Operand<Op::div>& operands)
		{
			printOperator(operands, "div");
		}

		void print(const Operand<Op::idiv>& operands)
		{
			printOperator(operands, "idiv");
		}


		void print(const Operand<Op::qualifiers>& operands)
		{
			line() << "qualifier %" << operands.lvid << ": ";
			out << (operands.flag ? '+' : '-');
			switch (operands.qualifier)
			{
				case IR::ISA::TypeQualifier::ref:      out << "ref"; break;
				case IR::ISA::TypeQualifier::constant: out << "const"; break;
			}
		}

		void printAtomInstanceID(uint32_t atomid, uint32_t instanceid)
		{
			if (atommap and atomid != 0)
			{
				out << " // ";
				auto caption = atommap->symbolname(atomid, instanceid);
				if (not caption.empty())
					out << caption;
				else
					out << "<not-found>";
			}
		}

		void print(const Operand<Op::assign>& operands)
		{
			line() << '%' << operands.lhs << " = assign %" << operands.rhs;
		}


		void print(const Operand<Op::fieldget>& operands)
		{
			line() << '%' << operands.lvid << " = fieldget u64 %" << operands.self;
			out << '.' << operands.var;
		}


		void print(const Operand<Op::fieldset>& operands)
		{
			line() << "fieldset %" << operands.self;
			out << '.' << operands.var;
			out << " = %" << operands.lvid;
		}


		void print(const Operand<Op::stackalloc>& operands)
		{
			line() << "alloca %" << operands.lvid;
			out << ": ";
			out << nytype_to_cstring((nytype_t) operands.type);

			if (operands.atomid != (uint32_t) -1)
				out << " atom: " << operands.atomid;
		}

		void print(const Operand<Op::storeConstant>& operands)
		{
			line() << '%' << operands.lvid << " = constant ";
			out << (void*) operands.value.u64;
			out << " (.u64: " << operands.value.u64 << ", .f64: " << operands.value.f64 << ')';
		}

		void print(const Operand<Op::storeText>& operands)
		{
			line() << '%' << operands.lvid << " = text ";
			printString(operands.text);
		}

		void print(const Operand<Op::store>& operands)
		{
			line() << '%' << operands.lvid << " = %" << operands.source;
		}

		void print(const Operand<Op::ret>& operands)
		{
			if (operands.lvid == 0)
				line() << "return void";
			else
				line() << "return %" << operands.lvid << ", copy %" << operands.tmplvid;
		}

		void print(const Operand<Op::stacksize>& operands)
		{
			line() << "stack.size +" << operands.add;
		}


		void print(const Operand<Op::push>& operands)
		{
			if (operands.name == 0)
			{
				line() << "push %";
			}
			else
			{
				line() << "push ";
				printString(operands.name);
				out << " %";
			}
			out << operands.lvid;
		}

		void print(const Operand<Op::tpush>& operands)
		{
			if (operands.name == 0)
			{
				line() << "tpush %";
			}
			else
			{
				line() << "tpush ";
				printString(operands.name);
				out << " %";
			}
			out << operands.lvid;
		}

		void print(const Operand<Op::call>& operands)
		{
			line() << '%' << operands.lvid << " = call ";
			if (operands.instanceid == (uint32_t) -1)
			{
				out << '%' << operands.ptr2func;
			}
			else
			{
				out << "atom -> ";
				out << operands.ptr2func << " #" << operands.instanceid;
				printAtomInstanceID(operands.ptr2func, operands.instanceid);
			}
		}

		void print(const Operand<Op::intrinsic>& operands)
		{
			line() << '%' << operands.lvid << " = intrinsic ";
			if (operands.iid != (uint32_t) -1)
				out << "id:" << operands.iid;
			else
				printString(operands.intrinsic);
		}

		void print(const Operand<Op::comment>& operands)
		{
			if (operands.text != 0) // not empty
			{
				line() << "// ";
				auto text = sequence.stringrefs[operands.text];
				if (not text.contains('\n'))
				{
					printString(operands.text);
				}
				else
				{
					YString s = text;
					YString r;
					r << '\n' << lineHeader <<  tabs << "// ";
					s.replace("\n", r);
					s.trimRight();
					out << '@' << operands.text << ' ' << s;
				}
			}
			else
				printEOL();
		}

		void print(const Operand<Op::namealias>& operands)
		{
			line() << "alias ";
			printString(operands.name);
			out << " -> %" << operands.lvid;
		}


		void print(const Operand<Op::debugfile>& operands)
		{
			line() << "dbg source file '";
			printString(operands.filename);
			out << '\'';
		}

		void print(const Operand<Op::debugpos>& operands)
		{
			line() << "dbg l." << operands.line << ',' << operands.offset;
		}


		void print(const Operand<Op::self>& operands)
		{
			line() << "self %" << operands.self;
		}


		void print(const Operand<Op::identify>& operands)
		{
			line() << '%' << operands.lvid << " = identify ";
			if (operands.self != 0)
				out << '%' << operands.self << " . ";
			printString(operands.text);
		}

		void print(const Operand<Op::identifyset>& operands)
		{
			line() << '%' << operands.lvid << " = identify:set ";
			if (operands.self != 0)
				out << '%' << operands.self << " . ";
			printString(operands.text);
		}

		void print(const Operand<Op::ensureresolved>& operands)
		{
			line() << "ensure resolved %" << operands.lvid;
		}

		void print(const Operand<Op::commontype>& operands)
		{
			line() << '%' << operands.lvid << " = common type with %" << operands.previous;
		}

		void print(const Operand<Op::label>& operands)
		{
			line() << "label " << operands.label << ":";
		}


		void print(const Operand<Op::jmp>& operands)
		{
			line() << "jmp " << operands.label;
		}

		void print(const Operand<Op::jz>& operands)
		{
			line() << "jz %" << operands.lvid << " == 0, %" << operands.result;
			out << ", goto lbl " << operands.label;
		}

		void print(const Operand<Op::jnz>& operands)
		{
			line() << "jnz %" << operands.lvid << " != 0, %" << operands.result;
			out << ", goto lbl " << operands.label;
		}


		void print(const Operand<Op::scope>&)
		{
			line() << '{';
			indent();
		}

		void print(const Operand<Op::end>&)
		{
			unindent();
			line() << '}';

			// partial print and the end-of-scope has been reached
			if (tabs.empty() and offset != 0)
				sequence.invalidateCursor(*cursor);
		}

		void print(const Operand<Op::follow>& operands)
		{
			line() << '%';
			if (0 == operands.symlink)
				out << operands.follower << " follows -> %" << operands.lvid;
			else
				out << operands.lvid << " symlink -> %" << operands.follower; // print like 'ln' command
		}

		void print(const Operand<Op::classdefsizeof>& operands)
		{
			line() << '%' << operands.lvid << " = sizeof (%" << operands.type;
			out << " or atomid " << operands.type << ')';
		}

		void print(const Operand<Op::allocate>& operands)
		{
			line() << '%' << operands.lvid << " = allocate %" << operands.atomid;
			out << " or atomid " << operands.atomid;
		}


		void print(const Operand<Op::memalloc>& operands)
		{
			line() << '%' << operands.lvid << " = memory.allocate %" << operands.regsize << " bytes";
		}

		void print(const Operand<Op::memfree>& operands)
		{
			line() << "memory.free %" << operands.lvid << " size %" << operands.regsize;
		}

		void print(const Operand<Op::memfill>& operands)
		{
			line() << "memory.fill %" << operands.lvid << " size %" << operands.regsize;
		}

		void print(const Operand<Op::memcopy>& operands)
		{
			line() << "memory.copy %" << operands.lvid << " from {%";
			out << operands.srclvid;
			out << ", size %" << operands.regsize << '}';
		}

		void print(const Operand<Op::memmove>& operands)
		{
			line() << "memory.move %" << operands.lvid << " from {%";
			out << operands.srclvid;
			out << ", size %" << operands.regsize << '}';
		}

		void print(const Operand<Op::memcmp>& operands)
		{
			line() << '%' << operands.regsize << " = memory.cmp %" << operands.lvid << " from {%";
			out << operands.srclvid;
			out << ", size %" << operands.regsize << '}';
		}

		void print(const Operand<Op::cstrlen>& operands)
		{
			line() << '%' << operands.lvid << " = cstrlen" << operands.bits << " %" << operands.ptr;
		}

		void print(const Operand<Op::load_u64>& operands)
		{
			line() << '%' << operands.lvid << " = load __u64 %" << operands.ptrlvid;
		}
		void print(const Operand<Op::load_u32>& operands)
		{
			line() << '%' << operands.lvid << " = load __u32 %" << operands.ptrlvid;
		}

		void print(const Operand<Op::load_u8>& operands)
		{
			line() << '%' << operands.lvid << " = load __u8 %" << operands.ptrlvid;
		}

		void print(const Operand<Op::store_u64>& operands)
		{
			line() << "store __u64 %" << operands.ptrlvid << " = %" << operands.lvid;
		}

		void print(const Operand<Op::store_u32>& operands)
		{
			line() << "store __u32 %" << operands.ptrlvid << " = %" << operands.lvid;
		}

		void print(const Operand<Op::store_u8>& operands)
		{
			line() << "store __u8 %" << operands.ptrlvid << " = %" << operands.lvid;
		}


		void print(const Operand<Op::memrealloc>& operands)
		{
			line() << "memory.realloc %" << operands.lvid
				<< " oldsize %" << operands.oldsize
				<< " newsize %" << operands.newsize;
		}


		void print(const Operand<Op::ref>& operands)
		{
			line() << "+ref %" << operands.lvid;
		}

		void print(const Operand<Op::unref>& operands)
		{
			line() << "-unref %" << operands.lvid;
			if (operands.atomid != 0)
			{
				out << " {atom id:";
				out << operands.atomid << " #" << operands.instanceid << '}';
			}
		}

		void print(const Operand<Op::dispose>& operands)
		{
			line() << "dispose %" << operands.lvid;
			if (operands.atomid != 0)
			{
				out << " {atom id:";
				out << operands.atomid << " #" << operands.instanceid << '}';
			}
		}


		void print(const Operand<Op::typeisobject>& operands)
		{
			line() << "type is object %" << operands.lvid;
		}


		void print(const Operand<Op::inherit>& operands)
		{
			line() << '%' << operands.lhs << " inherits ";
			switch (operands.inherit)
			{
				case 2:
				{
					out << "qualifiers from %" << operands.rhs;
					break;
				}
				default:
					out << "<unknown>";
			}
		}


		void print(const Operand<Op::memcheckhold>& operands)
		{
			line() << "memchecker.hold %" << operands.lvid << ", size: %" << operands.size;
		}


		void print(const Operand<Op::opassert>& operands)
		{
			line() << "assert %" << operands.lvid << " != 0";
		}


		void print(const Operand<Op::blueprint>& operands)
		{
			auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
			switch (kind)
			{
				case ISA::Blueprint::funcdef:
				{
					printEOL();

					line();
					if (operands.lvid != 0)
						out << '%' << operands.lvid << " = ";
					out << "func id:";
					uint32_t  atomid = operands.atomid;
					if (atomid != (uint32_t) -1)
						out << atomid;
					else
						out << "<unspecified>";

					out << ' ';
					printString(operands.name);
					printEOL();
					line() << '{';
					indent();
					break;
				}
				case ISA::Blueprint::param:
				{
					line() << "param ";
					printString(operands.name);
					out << ": %" << static_cast<uint32_t>(operands.lvid);
					break;
				}
				case ISA::Blueprint::gentypeparam:
				{
					line() << "type param ";
					printString(operands.name);
					out << ": %" << static_cast<uint32_t>(operands.lvid);
					break;
				}
				case ISA::Blueprint::paramself:
				{
					line() << "param ";
					printString(operands.name);
					out << ", %" << (uint32_t) operands.lvid << " [self assign]";
					break;
				}
				case ISA::Blueprint::classdef:
				{
					printEOL();
					line();
					if (operands.lvid != 0)
						out << '%' << operands.lvid << " = ";
					out << "class ";

					printString(operands.name);
					printEOL();
					line() << '{';
					indent();
					printEOL();

					// ID
					uint32_t  atomid = operands.atomid;
					line() << "// atomid: ";
					if (atomid != (uint32_t) -1)
						out << atomid;
					else
						out << "<unspecified>";
					break;
				}
				case ISA::Blueprint::vardef:
				{
					line() << "var ";
					printString(operands.name);
					out << ": %" << (uint32_t) operands.lvid;
					break;
				}
				case ISA::Blueprint::typealias:
				{
					line();
					out << "typedef ";

					printString(operands.name);
					printEOL();
					line() << '{';
					indent();
					printEOL();

					// ID
					uint32_t  atomid = operands.atomid;
					line() << "// atomid: ";
					if (atomid != (uint32_t) -1)
						out << atomid;
					else
						out << "<unspecified>";
					break;
				}
				case ISA::Blueprint::namespacedef:
				{
					line() << "namespace ";
					printString(operands.name);
					break;
				}

				case ISA::Blueprint::unit:
				{
					line() << "unit ";
					printString(operands.name);
					printEOL();
					line() << '{';
					indent();
					break;
				}
			}
		}


		void print(const Operand<Op::pragma>& operands)
		{
			switch (operands.pragma)
			{
				case Pragma::codegen:
				{
					line() << "pragma codegen ";
					out << ((operands.value.codegen != 0) ? "enable" : "disable");
					break;
				}
				case Pragma::blueprintsize:
				{
					auto size = operands.value.blueprintsize;
					line() << "// blueprint size " << size << " opcodes (";
					out << (size * sizeof(Instruction)) << " bytes)";
					break;
				}
				case Pragma::visibility:
				{
					auto* text = nyvisibility_to_cstring((nyvisibility_t) operands.value.visibility);
					line() << "pragma visibility " << text;
					break;
				}
				case Pragma::bodystart:
				{
					line() << "pragma body start";
					break;
				}
				case Pragma::shortcircuit:
				{
					line() << "pragma shortcircuit ";
					out << ((operands.value.shortcircuit) ? "__true" : "__false");
					break;
				}
				case Pragma::shortcircuitOpNopOffset:
				{
					line() << "pragma shortcircuit metadata: label: ";
					out << operands.value.shortcircuitMetadata.label;
					out << ", tmpvar: ";
					out << (1 + operands.value.shortcircuitMetadata.label);
					break;
				}
				case Pragma::shortcircuitMutateToBool:
				{
					line() << "pragma shortcircuit mutate to bool: %";
					out << operands.value.shortcircuitMutate.lvid;
					out << " = new bool %" << operands.value.shortcircuitMutate.source;
					break;
				}
				case Pragma::builtinalias:
				{
					line() << "pragma builtinalias ";
					printString(operands.value.builtinalias.namesid);
					break;
				}
				case Pragma::suggest:
				{
					line() << "pragma suggest ";
					out << (operands.value.suggest != 0 ? "true" : "false");
					break;
				}
				case Pragma::synthetic:
				{
					line() << "pragma synthetic %" << operands.value.synthetic.lvid << " = ";
					out << (operands.value.synthetic.onoff != 0 ? "true" : "false");
					break;
				}
				case Pragma::unknown:
				{
					line() << "<invalid pragma identifier>";
					break;
				}
			}
		}


		template<ny::IR::ISA::Op O> inline void visit(const Operand<O>& instr)
		{
			print(instr);
			printEOL();
		}

		inline void visit(const IR::Instruction& instruction)
		{
			LIBNANYC_IR_VISIT_SEQUENCE(const IR::ISA::Operand, *this, instruction);
		}
	};







} // anonymous namespace
} // namespace ISA
} // namespace IR
} // namespace ny
