#pragma once
#include "opcodes.h"
#include "../program.h"
#include <stdio.h>
#include <stdlib.h>
#include "details/atom/atom-map.h"



namespace Nany
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
		const Program& program;
		const Instruction** cursor = nullptr;
		const Nany::AtomMap* atommap = nullptr;


		Printer(S& out, const Program& program) : out(out), program(program) {}
		void indent()   { tabs.append("    ", 4); }
		void unindent() { tabs.chop(4); }
		void printEOL() { out += '\n'; }

		void printString(uint32_t sid)
		{
			auto text = program.stringrefs[sid];
			out << '@' << sid << "\"" << text << "\"";
		}


		void print(const Operand<Op::nop>&)
		{
			out << tabs << "nop";
		}


		template<class T> void printOperator(const T& operands, const AnyString& opname)
		{
			out << tabs << '%' << operands.lvid << " = %" << operands.lhs << ' ' << opname << " %" << operands.rhs;
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
			out << tabs << "qualifier %" << operands.lvid << ": ";
			out << (operands.flag ? '+' : '-');
			switch (operands.qualifier)
			{
				case 1: out << "ref"; break;
				case 2: out << "const"; break;
			}
		}

		void printAtomInstanceID(uint32_t atomid, uint32_t instanceid)
		{
			if (atommap and atomid != 0)
			{
				out << " // ";
				auto caption = atommap->fetchProgramCaption(atomid, instanceid);
				if (not caption.empty())
					out << caption;
				else
					out << "<not-found>";
			}
		}


		void print(const Operand<Op::assign>& operands)
		{
			out << tabs << '%' << operands.lhs << " = assign %" << operands.rhs;
		}


		void print(const Operand<Op::fieldget>& operands)
		{
			out << tabs << '%' << operands.lvid << " = fieldget u64 %" << operands.self;
			out << '.' << operands.var;
		}


		void print(const Operand<Op::fieldset>& operands)
		{
			out << tabs << "fieldset %" << operands.self;
			out << '.' << operands.var;
			out << " = %" << operands.lvid;
		}


		void print(const Operand<Op::stackalloc>& operands)
		{
			out << tabs << "alloca %" << operands.lvid;
			out << ": ";
			out << nany_type_to_cstring((nytype_t) operands.type);

			if (operands.atomid != (uint32_t) -1)
				out << " atom: " << operands.atomid;
		}

		void print(const Operand<Op::storeConstant>& operands)
		{
			out << tabs << '%' << operands.lvid << " = constant ";
			out << (void*) operands.value.u64;
			out << " (u64: " << operands.value.u64 << ", f64: " << operands.value.f64 << ')';
		}

		void print(const Operand<Op::storeText>& operands)
		{
			out << tabs << '%' << operands.lvid << " = text ";
			printString(operands.text);
		}

		void print(const Operand<Op::store>& operands)
		{
			out << tabs << '%' << operands.lvid << " = %" << operands.source;
		}

		void print(const Operand<Op::ret>& operands)
		{
			out << tabs << "return %" << operands.lvid;
		}

		void print(const Operand<Op::stacksize>& operands)
		{
			out << tabs << "stack.size +" << operands.add;
		}


		void print(const Operand<Op::push>& operands)
		{
			if (operands.name == 0)
			{
				out << tabs << "push %";
			}
			else
			{
				out << tabs << "push ";
				printString(operands.name);
				out << " %";
			}
			out << operands.lvid;
		}

		void print(const Operand<Op::call>& operands)
		{
			out << tabs << '%' << operands.lvid << " = call ";
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
			out << tabs << '%' << operands.lvid << " = intrinsic ";
			printString(operands.intrinsic);
		}

		void print(const Operand<Op::comment>& operands)
		{
			if (operands.text != 0) // not empty
			{
				out << tabs << "// ";
				auto text = program.stringrefs[operands.text];
				if (not text.contains('\n'))
				{
					printString(operands.text);
				}
				else
				{
					YString s = text;
					YString r;
					r << '\n' << tabs << "// ";
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
			out << tabs << "alias ";
			printString(operands.name);
			out << " -> %" << operands.lvid;
		}


		void print(const Operand<Op::debugfile>& operands)
		{
			out << tabs << "dbg source file '";
			printString(operands.filename);
			out << '\'';
		}

		void print(const Operand<Op::debugpos>& operands)
		{
			out << tabs << "dbg l." << operands.line << ',' << operands.offset;
		}


		void print(const Operand<Op::self>& operands)
		{
			out << tabs << "self %" << operands.self;
		}


		void print(const Operand<Op::identify>& operands)
		{
			out << tabs << '%' << operands.lvid << " = identify ";
			if (operands.self != 0)
				out << '%' << operands.self << " . ";
			printString(operands.text);
		}


		void print(const Operand<Op::label>& operands)
		{
			out << tabs << "label " << operands.label << ":";
		}


		void print(const Operand<Op::jmp>& operands)
		{
			out << tabs << "jmp " << operands.label;
		}

		void print(const Operand<Op::jz>& operands)
		{
			out << tabs << "jz %" << operands.lvid << " == 0, %" << operands.result;
			out << ", goto lbl " << operands.label;
		}

		void print(const Operand<Op::jnz>& operands)
		{
			out << tabs << "jnz %" << operands.lvid << " != 0, %" << operands.result;
			out << ", goto lbl " << operands.label;
		}



		void print(const Operand<Op::scope>&)
		{
			out << tabs << '{';
			indent();
		}

		void print(const Operand<Op::end>&)
		{
			unindent();
			out << tabs << '}';
		}

		void print(const Operand<Op::follow>& operands)
		{
			out << tabs << '%';
			if (0 == operands.symlink)
				out << operands.follower << " follows -> %" << operands.lvid;
			else
				out << operands.lvid << " symlink -> %" << operands.follower; // print like 'ln' command
		}

		void print(const Operand<Op::classdefsizeof>& operands)
		{
			out << tabs << '%' << operands.lvid << " = sizeof (%" << operands.type << " or atomid " << operands.type << ')';
		}

		void print(const Operand<Op::allocate>& operands)
		{
			out << tabs << '%' << operands.lvid << " = allocate %" << operands.atomid << " or atomid " << operands.atomid;
		}


		void print(const Operand<Op::memalloc>& operands)
		{
			out << tabs << '%' << operands.lvid << " = memory.allocate %" << operands.regsize << " bytes";
		}

		void print(const Operand<Op::memfree>& operands)
		{
			out << tabs << "memory.free %" << operands.lvid << " size %" << operands.regsize;
		}


		void print(const Operand<Op::ref>& operands)
		{
			out << tabs << "+ref %" << operands.lvid;
		}

		void print(const Operand<Op::unref>& operands)
		{
			out << tabs << "-unref %" << operands.lvid;
			if (operands.atomid != 0)
			{
				out << " {atom id:";
				out << operands.atomid << " #" << operands.instanceid << '}';
			}
		}

		void print(const Operand<Op::dispose>& operands)
		{
			out << tabs << "dispose %" << operands.lvid;
			if (operands.atomid != 0)
			{
				out << " {atom id:";
				out << operands.atomid << " #" << operands.instanceid << '}';
			}
		}


		void print(const Operand<Op::typeisobject>& operands)
		{
			out << tabs << "type is object %" << operands.lvid;
		}


		void print(const Operand<Op::inherit>& operands)
		{
			out << tabs << '%' << operands.lhs << " inherits ";
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


		void print(const Operand<Op::pragma>& operands)
		{
			if (operands.pragma < static_cast<uint32_t>(Pragma::max))
			{
				switch ((Pragma) operands.pragma)
				{
					case Pragma::codegen:
					{
						out << tabs << "pragma codegen " << ((operands.value.codegen != 0) ? "enable" : "disable");
						break;
					}
					case Pragma::namespacedef:
					{
						out << tabs << "namespace ";
						printString(operands.value.namespacedef);
						break;
					}
					case Pragma::blueprintclassdef:
					{
						printEOL();
						out << tabs << "class ";
						printString(operands.value.blueprint.name);
						printEOL();
						out << tabs << '{';
						indent();
						printEOL();

						// ID
						uint32_t  atomid = operands.value.blueprint.atomid;
						out << tabs << "// atomid: ";
						if (atomid != (uint32_t) -1)
							out << atomid;
						else
							out << "<unspecified>";
						break;
					}
					case Pragma::blueprintfuncdef:
					{
						printEOL();

						out << tabs << "func id:";
						uint32_t  atomid = operands.value.blueprint.atomid;
						if (atomid != (uint32_t) -1)
							out << atomid;
						else
							out << "<unspecified>";

						out << ' ';
						printString(operands.value.blueprint.name);
						printEOL();
						out << tabs << '{';
						indent();
						break;
					}

					case Pragma::blueprintparamself:
					{
						out << tabs << "self param ";
						printString(operands.value.param.name);
						out << ", %" << operands.value.param.lvid;
						break;
					}

					case Pragma::blueprintparam:
					{
						out << tabs << "param ";
						printString(operands.value.param.name);
						out << ": %" << operands.value.param.lvid;
						break;
					}
					case Pragma::blueprintvar:
					{
						out << tabs << "var ";
						printString(operands.value.vardef.name);
						out << ": %" << operands.value.vardef.lvid;
						break;
					}
					case Pragma::blueprintsize:
					{
						auto size = operands.value.blueprintsize;
						out << tabs << "// blueprint size " << size << " opcodes (";
						out << (size * sizeof(Instruction)) << " bytes)";
						break;
					}
					case Pragma::visibility:
					{
						auto* text = nany_visibility_to_cstring((nyvisibility_t) operands.value.visibility);
						out << tabs << "pragma visibility " << text;
						break;
					}
					case Pragma::bodystart:
					{
						out << tabs << "pragma body start";
						break;
					}

					case Pragma::shortcircuit:
					{
						out << tabs << "pragma shortcircuit ";
						out << ((operands.value.shortcircuit) ? "__true" : "__false");
						break;
					}
					case Pragma::shortcircuitOpNopOffset:
					{
						out << tabs << "pragma shortcircuit metadata: label: ";
						out << operands.value.shortcircuitMetadata.label;
						out << ", tmpvar: ";
						out << (1 + operands.value.shortcircuitMetadata.label);
						break;
					}

					case Pragma::builtinalias:
					{
						out << tabs << "pragma builtinalias ";
						printString(operands.value.builtinalias.namesid);
						break;
					}

					default: out << tabs << "<unknown pragma>";
				}
			}
			else
				out << tabs << "<invalid pragma identifier>";
		}



		template<enum Op O> inline void visit(const Operand<O>& instr)
		{
			print(instr);
			printEOL();
		}

		inline void visit(const IR::Instruction& instruction)
		{
			LIBNANY_IR_VISIT_PROGRAM(const IR::ISA::Operand, *this, instruction);
		}
	};







} // anonymous namespace
} // namespace ISA
} // namespace IR
} // namespace Nany
