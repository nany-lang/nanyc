#include "details/context/isolate.h"
#include "details/atom/classdef-table.h"
#include "details/reporting/report.h"
#include "libnany-config.h"
#include <utility>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace Yuni;




namespace Nany
{

	namespace // anonymous
	{

		//! Stack frame per atom definition (class, func)
		struct AtomStackFrame final
		{
			AtomStackFrame(Atom& atom): atom(atom) {}
			//! The current atom
			Atom& atom;
			//! The current scope depth for the current stack frame
			uint scope = 0;
			uint parameterIndex = 0;
			//! Convenient classdefs alias
			std::vector<CLID> classdefs;
		};



		class ProgramSynchronizer final
		{
		public:
			ProgramSynchronizer(Logs::Report& report, Isolate& isolate, IR::Program& program)
				: isolate(isolate)
				, currentProgram(program)
				, report(report)
			{
				atomStack.reserve(4); // arbitrary

				// root atom
				// the address of the root atom is always the same. No need to lock the isolate
				atomStack.emplace_back(isolate.classdefTable.atoms.root);
				// reduce memory allocations
				lastPushedNamedParameters.reserve(8); // arbitrary
				lastPushedIndexedParameters.reserve(8);
			}


			Logs::Report error()
			{
				success = false;
				auto err = report.error();
				err.message.origins.location.filename   = currentFilename;
				err.message.origins.location.pos.line   = currentLine;
				err.message.origins.location.pos.offset = currentOffset;
				return err;
			}


			template<IR::ISA::Op O> void printError(const IR::ISA::Operand<O>& operands, AnyString msg = nullptr)
			{
				// example: ICE: unknown opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
				auto trace = report.ICE();
				if (not msg.empty())
					trace << msg;
				else
					trace << "attach program: unknown opcode: '" << IR::ISA::Operand<O>::opname() << '\'';

				trace << ": from '" << IR::ISA::print(currentProgram, operands) << '\'';
				success = false;
			}


			template<IR::ISA::Op O> bool checkForLVID(const IR::ISA::Operand<O>& operands, LVID lvid)
			{
				if (unlikely(lvid == 0 or not (lvid < atomStack.back().classdefs.size())))
				{
					printError(operands, String{"invalid lvid %"} << lvid << " (upper bound: %" << atomStack.back().classdefs.size() << ')');
					return false;
				}
				return true;
			}


			inline void resetClassdefOriginFromCurrentPosition(Classdef& cdef)
			{
				cdef.origins.line = currentLine;
				cdef.origins.offset = currentOffset;
				cdef.origins.filename = currentFilename;
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
			{
				assert(not atomStack.empty());
				if (atomStack.empty())
					return printError(operands, "invalid stack for blueprint param");

				if (unlikely(not (operands.pragma < static_cast<uint32_t>(IR::ISA::Pragma::max))))
					return printError(operands, "invalid pragma");

				switch ((IR::ISA::Pragma) operands.pragma)
				{
					case IR::ISA::Pragma::codegen:
					{
						break;
					}

					case IR::ISA::Pragma::namespacedef:
					{
						AnyString nmname = currentProgram.stringrefs[operands.value.namespacedef];
						assert(not atomStack.empty());
						Atom& parentAtom = atomStack.back().atom;

						MutexLocker locker{isolate.mutex};
						Atom* newRoot = isolate.classdefTable.atoms.createNamespace(parentAtom, nmname);
						assert(newRoot != nullptr);
						newRoot->usedDefined = true;
						// create a pseudo classdef to easily retrieve the real atom from a clid
						isolate.classdefTable.registerAtom(newRoot);
						atomStack.push_back(AtomStackFrame{*newRoot});
						break;
					}

					case IR::ISA::Pragma::blueprintvar:
					{
						assert(not atomStack.empty());
						// registering the blueprint into the outline...
						Atom& atom = atomStack.back().atom;
						AnyString varname = currentProgram.stringrefs[operands.value.vardef.name];
						if (unlikely(varname.empty()))
							return printError(operands, "invalid func name");
						if (unlikely(atom.type != Atom::Type::classdef))
							return printError(operands, "vardef: invalid parent atom");

						MutexLocker locker{isolate.mutex};
						// create a new atom in the global type table
						auto* newVarAtom = isolate.classdefTable.atoms.createVardef(atom, varname);
						assert(newVarAtom != nullptr);
						newVarAtom->usedDefined      = true;

						isolate.classdefTable.registerAtom(newVarAtom);
						newVarAtom->returnType.clid.reclass(atom.atomid, operands.value.vardef.lvid);
						break;
					}

					case IR::ISA::Pragma::blueprintfuncdef:
					{
						assert(not atomStack.empty());
						// registering the blueprint into the outline...
						Atom& atom = atomStack.back().atom;
						AnyString funcname = currentProgram.stringrefs[operands.value.blueprint.name];
						if (unlikely(funcname.empty()))
							return printError(operands, "invalid func name");

						// reset last lvid and parameters
						lastLVID = 0;
						lastPushedNamedParameters.clear();
						lastPushedIndexedParameters.clear();

						MutexLocker locker{isolate.mutex};
						// create a new atom in the global type table
						auto* newFuncAtom = isolate.classdefTable.atoms.createFuncdef(atom, funcname);
						assert(newFuncAtom != nullptr);
						newFuncAtom->usedDefined      = true;
						newFuncAtom->opcodes.program  = &currentProgram;
						newFuncAtom->opcodes.offset   = currentProgram.offsetOf(operands);
						// create a pseudo classdef to easily retrieve the real atom from a clid
						isolate.classdefTable.registerAtom(newFuncAtom);

						operands.value.blueprint.atomid = newFuncAtom->atomid;

						// return type
						newFuncAtom->returnType.clid.reclass(newFuncAtom->atomid, 1);

						// requires additional information
						needAtomDbgFileReport = true;
						needAtomDbgOffsetReport = true;
						atomStack.push_back(AtomStackFrame{*newFuncAtom});
						break;
					}

					case IR::ISA::Pragma::blueprintparam:
					case IR::ISA::Pragma::blueprintparamself:
					{
						assert(not atomStack.empty());
						auto& frame = atomStack.back();
						// calculating the lvid for the current parameter
						// (+1 since %1 is the return value/type)
						uint paramLVID = (++frame.parameterIndex) + 1;

						if (unlikely(not checkForLVID(operands, paramLVID)))
							return;

						CLID clid {frame.atom.atomid, paramLVID};
						AnyString name = currentProgram.stringrefs[operands.value.param.name];
						frame.atom.parameters.append(clid, name);

						// keep somewhere that this definition is a variable instance
						MutexLocker locker{isolate.mutex};
						auto& cdef = isolate.classdefTable.classdef(clid);
						cdef.instance = true;
						cdef.qualifiers.ref = false; // should not be 'ref' by default, contrary to all other classdefs
						break;
					}

					case IR::ISA::Pragma::blueprintclassdef:
					{
						assert(not atomStack.empty());
						// registering the blueprint into the outline...
						Atom& atom = atomStack.back().atom;

						// reset last lvid and parameters
						lastLVID = 0;
						lastPushedNamedParameters.clear();
						lastPushedIndexedParameters.clear();

						AnyString classname = currentProgram.stringrefs[operands.value.blueprint.name];

						MutexLocker locker{isolate.mutex};
						// create a new atom in the global type table
						auto* newClassAtom = isolate.classdefTable.atoms.createClassdef(atom, classname);
						assert(newClassAtom != nullptr);
						newClassAtom->usedDefined     = true;
						newClassAtom->opcodes.program = &currentProgram;
						newClassAtom->opcodes.offset  = currentProgram.offsetOf(operands);
						// create a pseudo classdef to easily retrieve the real atom from a clid
						isolate.classdefTable.registerAtom(newClassAtom);
						// update atomid
						operands.value.blueprint.atomid = newClassAtom->atomid;

						// requires additional information
						needAtomDbgFileReport = true;
						needAtomDbgOffsetReport = true;
						atomStack.push_back(AtomStackFrame{*newClassAtom});
						break;
					}

					case IR::ISA::Pragma::builtinalias:
					{
						assert(not atomStack.empty());
						Atom& atom = atomStack.back().atom;
						atom.builtinalias = currentProgram.stringrefs[operands.value.builtinalias.namesid];
						break;
					}

					case IR::ISA::Pragma::shortcircuit:
					{
						assert(not atomStack.empty());
						atomStack.back().atom.parameters.shortcircuitValue = (0 != operands.value.shortcircuit);
						break;
					}
					case IR::ISA::Pragma::suggest:
					{
						assert(not atomStack.empty());
						atomStack.back().atom.canBeSuggestedInErrReporting = (0 != operands.value.suggest);
						break;
					}

					case IR::ISA::Pragma::blueprintsize:
					case IR::ISA::Pragma::visibility:
					case IR::ISA::Pragma::bodystart:
					case IR::ISA::Pragma::shortcircuitOpNopOffset:
					case IR::ISA::Pragma::unknown:
					case IR::ISA::Pragma::max:
					{
						break;
					}
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::stacksize>& operands)
			{
				Atom& parentAtom = atomStack.back().atom;

				// creating all related classdefs
				// (take max with 1 to prevent against invalid opcode)
				uint localvarCount = operands.add;

				MutexLocker locker{isolate.mutex};
				parentAtom.localVariablesCount = localvarCount;
				isolate.classdefTable.bulkCreate(atomStack.back().classdefs, parentAtom.atomid, localvarCount);

				if (parentAtom.type == Atom::Type::funcdef)
				{
					// creating all related classdefs
					// (take max with 1 to prevent against invalid opcode)
					if (localvarCount == 0)
						return printError(operands, "invalid local variable count for a func blueprint");

					// like parameters, the return type should not 'ref' by default
					isolate.classdefTable.classdef(CLID{parentAtom.atomid, 1}).qualifiers.ref = false;
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::scope>&)
			{
				if (unlikely(not (atomStack.size() > 0)))
					throw (String{} << currentFilename << ':' << currentLine << ": invalid stack");

				if (not atomStack.empty())
					++(atomStack.back().scope);

				lastLVID = 0;
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::end>&)
			{
				// reset the last lvid
				lastLVID = 0;
				// pop the stack

				if (unlikely(not (atomStack.size() > 1)))
					throw (String{} << currentFilename << ':' << currentLine << ": invalid stack");

				if (likely(not atomStack.empty()))
				{
					auto& scope = atomStack.back().scope;
					// the scope might be zero if the opcode 'end' comes from a class or a func
					if (scope > 0)
					{
						--scope;
					}
					else
					{
						// remove a part of the stack
						atomStack.pop_back();
					}
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;
				lastLVID = operands.lvid;
				auto clid = atomStack.back().classdefs[operands.lvid];

				MutexLocker locker{isolate.mutex};
				auto& cdef = isolate.classdefTable.classdef(clid);
				resetClassdefOriginFromCurrentPosition(cdef);
				switch ((nytype_t) operands.type)
				{
					default:
					{
						cdef.mutateToBuiltin((nytype_t) operands.type);
						cdef.instance = true; // keep somewhere that this definition is a variable instance
						break;
					}
					case nyt_void: cdef.mutateToVoid(); break;
					case nyt_any:  cdef.mutateToAny(); break;
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::self>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.self)))
					return;
				if (unlikely(atomStack.empty()))
					return printError(operands, "invalid atom stack for 'resolveAsSelf'");

				// we can have at least 2 patterns:
				//
				//  * the most frequent, called from a method contained within a class
				//  * from the class itself, most likely a variable
				for (auto rit = atomStack.rbegin(); rit != atomStack.rend(); ++rit)
				{
					if (rit->atom.isClass())
					{
						// clid of the target variable
						CLID clid {atomStack.back().atom.atomid, operands.self};

						MutexLocker locker{isolate.mutex};
						auto& cdef = isolate.classdefTable.classdef(clid);
						resetClassdefOriginFromCurrentPosition(cdef);
						cdef.mutateToAtom(&(rit->atom));
						cdef.qualifiers.ref = true;
						return;
					}
				}

				// fallback - error
				printError(operands, "failed to find parent class for 'resolveAsSelf'");
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::identify>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;

				if (unlikely(operands.text == 0))
					return printError(operands, "invalid symbol name");
				AnyString name = currentProgram.stringrefs[operands.text];

				lastLVID = operands.lvid;
				auto& atomFrame = atomStack.back();
				auto& localClassdefs = atomFrame.classdefs;

				assert(operands.lvid < localClassdefs.size());
				auto& clid = localClassdefs[operands.lvid];


				if (operands.self == 0)
				{
					// directly resolving a symbol accessible from the current scope
					// in this pass, we will only resolve local variables (and parameters)
					// all function calls must be resolved later

					MutexLocker locker{isolate.mutex};
					// will see later - currently unknown
					// classdef.mutateToAny();
					isolate.classdefTable.addClassdefInterfaceSelf(clid, name);
					resetClassdefOriginFromCurrentPosition(isolate.classdefTable.classdef(clid));
				}
				else
				{
					// trying to resolve an attribute
					// (aka `parent.current`)
					if (not checkForLVID(operands, operands.self))
						return;

					// the type is currently unknown
					// classdef.mutateToAny();
					// retrieving the parent classdef
					assert(operands.self < localClassdefs.size());
					auto& selfClassdef = localClassdefs[operands.self];

					MutexLocker locker{isolate.mutex};
					auto& funcdef = isolate.classdefTable.addClassdefInterface(selfClassdef, name);
					funcdef.clid  = clid;
					auto& cdef = isolate.classdefTable.classdef(clid);
					cdef.parentclid = selfClassdef;
					resetClassdefOriginFromCurrentPosition(cdef);
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::push>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;

				if (operands.name != 0) // named parameter
				{
					AnyString name = currentProgram.stringrefs[operands.name];
					lastPushedNamedParameters.emplace_back(std::make_pair(name, operands.lvid));
				}
				else
				{
					if (unlikely(not lastPushedNamedParameters.empty()))
						return printError(operands, "named parameters must be provided after standard parameters");

					lastPushedIndexedParameters.emplace_back(operands.lvid);
				}
			}


			inline void attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.ptr2func)))
					return;
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;

				auto& stackframe = atomStack.back();
				auto atomid      = stackframe.atom.atomid;
				auto& clidFuncToCall = stackframe.classdefs[operands.ptr2func];
				auto& clidRetValue   = stackframe.classdefs[operands.lvid];


				// update all underlying types
				{
					MutexLocker locker{isolate.mutex};

					resetClassdefOriginFromCurrentPosition(isolate.classdefTable.classdef(clidRetValue));

					auto& funcdef = isolate.classdefTable.addClassdefInterfaceSelf(clidFuncToCall);
					funcdef.resetReturnType(clidRetValue);
					funcdef.clid = clidFuncToCall;
					CLID clid{atomid, 0};

					for (uint i = 0; i != lastPushedIndexedParameters.size(); ++i)
					{
						auto lvid = lastPushedIndexedParameters[i];
						clid.reclass(lvid);
						funcdef.appendParameter(clid);

						auto& followup = isolate.classdefTable.classdef(clid).followup;
						followup.pushedIndexedParams.push_back(std::make_pair(clidFuncToCall, i));
					}

					for (auto& pair: lastPushedNamedParameters)
					{
						clid.reclass(pair.second);
						funcdef.appendParameter(pair.first, clid);

						auto& followup = isolate.classdefTable.classdef(clid).followup;
						followup.pushedNamedParams.push_back(std::make_pair(clidFuncToCall, pair.first));
					}
				}
			}

			void visit(IR::ISA::Operand<IR::ISA::Op::call>& operands)
			{
				attachFuncCall(operands);
				lastLVID = operands.lvid;
				lastPushedIndexedParameters.clear();
				lastPushedNamedParameters.clear();
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::ret>& operands)
			{
				if (operands.lvid != 0)
				{
					if (unlikely(not checkForLVID(operands, operands.lvid)))
						return;

					auto& stackframe = atomStack.back();
					auto& classdefRetValue = stackframe.classdefs[1]; // 1 is the return value
					auto& classdef = stackframe.classdefs[operands.lvid];

					MutexLocker locker{isolate.mutex};
					auto& followup = isolate.classdefTable.classdef(classdefRetValue).followup;
					followup.extends.push_back(classdef);
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::follow>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;
				if (unlikely(not checkForLVID(operands, operands.follower)))
					return;

				auto& stackframe = atomStack.back();
				auto& clidFollower = stackframe.classdefs[operands.follower];
				auto& clid = stackframe.classdefs[operands.lvid];

				MutexLocker locker{isolate.mutex};
				if (operands.symlink)
					isolate.classdefTable.makeHardlink(clid, clidFollower);
				else
					isolate.classdefTable.classdef(clidFollower).followup.extends.push_back(clid);
			}


			/*
			void visit(IR::ISA::Operand<IR::ISA::Op::allocate>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;
				if (unlikely(not checkForLVID(operands, operands.typeFrom)))
					return;

				auto& stackframe = atomStack.back();
				auto& clid = stackframe.classdefs[operands.lvid];
				auto& typeClid = stackframe.classdefs[operands.typeFrom];

				MutexLocker locker{isolate.mutex};
				auto& classdef = isolate.classdefTable.classdef(clid);
				classdef.instance = true; // mark as a variable
				classdef.followup.extends.push_back(typeClid);
			}*/


			void visit(IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::debugfile>& operands)
			{
				currentFilename = currentProgram.stringrefs[operands.filename].c_str();
				if (needAtomDbgFileReport)
				{
					needAtomDbgFileReport = false;
					atomStack.back().atom.origin.filename = currentFilename;
				}
			}

			void visit(IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
			{
				currentLine = operands.line;
				currentOffset = operands.offset;

				if (unlikely(needAtomDbgOffsetReport))
				{
					needAtomDbgOffsetReport = false;
					atomStack.back().atom.origin.line = currentLine;
					atomStack.back().atom.origin.offset = currentOffset;
				}
			}


			void visit(IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
			{
				auto& frame = atomStack.back();
				CLID clid {frame.atom.atomid, operands.lvid};
				bool onoff = (operands.flag != 0);

				switch (operands.qualifier)
				{
					case 1: // ref
					{
						MutexLocker locker{isolate.mutex};
						isolate.classdefTable.classdef(clid).qualifiers.ref = onoff;
						break;
					}
					case 2: // const
					{
						MutexLocker locker{isolate.mutex};
						isolate.classdefTable.classdef(clid).qualifiers.constant = onoff;
						break;
					}
					default:
						printError(operands, "invalid qualifier value");
				}
			}


			template<IR::ISA::Op O> void visit(IR::ISA::Operand<O>& operands)
			{
				switch (O)
				{
					case IR::ISA::Op::allocate:
					case IR::ISA::Op::comment:
					case IR::ISA::Op::classdefsizeof:
					case IR::ISA::Op::namealias:
					case IR::ISA::Op::store:
					case IR::ISA::Op::storeText:
					case IR::ISA::Op::storeConstant:
					case IR::ISA::Op::memalloc:
					case IR::ISA::Op::typeisobject:
					case IR::ISA::Op::ref:
					case IR::ISA::Op::unref:
					case IR::ISA::Op::assign:
					case IR::ISA::Op::inherit:
					case IR::ISA::Op::label:
					case IR::ISA::Op::jmp:
					case IR::ISA::Op::jz:
					case IR::ISA::Op::jnz:
					case IR::ISA::Op::nop:
						break;
					default:
						printError(operands);
						break;
				}
			}


		public:
			//! Isolate
			Isolate& isolate;
			//! Blueprint root element
			std::vector<AtomStackFrame> atomStack;
			//! Last lvid (for pushed parameters)
			LVID lastLVID = 0;
			//! Last pushed indexed parameters
			std::vector<LVID> lastPushedIndexedParameters;
			//! Last pushed named parameters
			std::vector<std::pair<AnyString, LVID>> lastPushedNamedParameters;
			//! exit status
			bool success = true;
			//! Current program
			IR::Program& currentProgram;
			//! Step
			Logs::Report report;

			const char* currentFilename = nullptr;
			uint currentLine = 0;
			uint currentOffset = 0;
			bool needAtomDbgFileReport = false;
			bool needAtomDbgOffsetReport = false;

			IR::Instruction** cursor = nullptr;

		}; // class ProgramSynchronizer



	} // anonymous namespace







	bool Isolate::attach(IR::Program& program, Logs::Report& report, bool owned)
	{
		// keep the program somewhere
		{
			MutexLocker locker{mutex};
			pAttachedPrograms.push_back(AttachedProgramRef{&program, owned});
		}
		ProgramSynchronizer syncer{report, *this, program};
		program.each(syncer);
		return syncer.success;
	}




} // namespace Nany
