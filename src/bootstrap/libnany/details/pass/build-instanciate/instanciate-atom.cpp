#include "instanciate.h"
#include "details/context/isolate.h"
#include <memory>
#include "details/reporting/report.h"
#include "details/atom/func-overload-match.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "libnany-traces.h"

using namespace Yuni;






namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	namespace // anonymous
	{

		struct PostProcessStackAllocWalker final
		{
			PostProcessStackAllocWalker(ClassdefTableView& table, uint32_t atomid)
				: table(table)
				, atomid(atomid)
			{}

			void visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>& opc)
			{
				auto& cdef = table.classdef(CLID{atomid, opc.lvid});
				if (not cdef.isBuiltinOrVoid())
				{
					auto* atom = table.findClassdefAtom(cdef);
					if (atom != nullptr)
					{
						assert(opc.atomid == 0 or opc.atomid == (uint32_t) -1 or opc.atomid == atom->atomid);
						opc.type = static_cast<uint32_t>(nyt_pointer);
						opc.atomid = atom->atomid;
					}
				}
				else
					opc.type = static_cast<uint32_t>(cdef.kind);
			}

			template<IR::ISA::Op O> void visit(const IR::ISA::Operand<O>&)
			{
				// nothing to do
			}

			ClassdefTableView& table;
			uint32_t atomid;
			IR::Instruction** cursor = nullptr;
		};

		static void postProcessStackAlloc(IR::Sequence& out, ClassdefTableView& table, uint32_t atomid)
		{
			PostProcessStackAllocWalker walker{table, atomid};
			out.each(walker);
		}


	} // anonymous namespace






	bool SequenceBuilder::instanciateAtomClassClone(Atom& atom, uint32_t lvid, uint32_t rhs)
	{
		assert(atom.isClass());
		if (unlikely(atom.hasErrors))
			return false;

		// first, try to find the user-defined clone function (if any)
		Atom* userDefinedClone = nullptr;
		switch (atom.findFuncAtom(userDefinedClone, "^clone"))
		{
			case 0:  break;
			case 1:
			{
				assert(userDefinedClone != nullptr);
				uint32_t instanceid = (uint32_t) -1;
				if (not instanciateAtomFunc(instanceid, (*userDefinedClone), /*void*/0, /*self*/lvid, rhs))
					return false;
				break;
			}
			default: return complainMultipleDefinitions(atom, "operator 'clone'");
		}


		Atom* clone = nullptr;
		switch (atom.findFuncAtom(clone, "^obj-clone"))
		{
			case 1:
			{
				assert(clone != nullptr);
				uint32_t instanceid = (uint32_t) -1;
				if (instanciateAtomFunc(instanceid, (*clone), /*void*/0, /*self*/lvid, rhs))
				{
					atom.classinfo.clone.atomid     = clone->atomid;
					atom.classinfo.clone.instanceid = instanceid;
					return true;
				}
				break;
			}
			case 0:
			{
				return complainMissingOperator(atom, "clone");
			}
			default: return complainMultipleDefinitions(atom, "operator 'obj-clone'");
		}
		return false;
	}


	bool SequenceBuilder::instanciateAtomClassDestructor(Atom& atom, uint32_t lvid)
	{
		// if the IR code produced when transforming the AST is invalid,
		// a common scenario is that the code tries to destroy something (via unref)
		// which is not be a real class
		if (unlikely(not atom.isClass()))
			return (ICE() << "trying to call the destructor of a non-class atom");

		if (unlikely(atom.hasErrors)) // error already reported
			return false;

		// first, try to find the user-defined dtor function (if any)
		Atom* userDefinedDtor = nullptr;
		switch (atom.findFuncAtom(userDefinedDtor, "^dispose"))
		{
			case 0: break;
			case 1:
			{
				assert(userDefinedDtor != nullptr);
				uint32_t instanceid = static_cast<uint32_t>(-1);
				if (not instanciateAtomFunc(instanceid, (*userDefinedDtor), /*void*/0, /*self*/lvid))
					return false;
				break;
			}
			default: return complainMultipleDefinitions(atom, "operator 'dispose'");
		}

		Atom* dtor = nullptr;
		switch (atom.findFuncAtom(dtor, "^obj-dispose"))
		{
			case 1:
			{
				assert(dtor != nullptr);
				uint32_t instanceid = static_cast<uint32_t>(-1);
				if (instanciateAtomFunc(instanceid, (*dtor), /*void*/0, /*self*/lvid))
				{
					atom.classinfo.dtor.atomid     = dtor->atomid;
					atom.classinfo.dtor.instanceid = instanceid;
					return true;
				}
				break;
			}
			case 0:
			{
				return complainMissingOperator(atom, "obj-dispose");
			}
			default: return complainMultipleDefinitions(atom, "operator 'obj-dispose'");
		}
		return false;
	}


	bool SequenceBuilder::instanciateAtomClass(Atom& atom)
	{
		assert(atom.isClass());
		atom.classinfo.isInstanciated = true;

		// parameters for the signature
		decltype(FuncOverloadMatch::result.params)  params;
		Logs::Message::Ptr newReport;

		Pass::Instanciate::InstanciateData info{newReport, atom, cdeftable, context, params};
		info.parentAtom = &(frame->atom);
		info.shouldMergeLayer = true;

		auto* sequence = Pass::Instanciate::InstanciateAtom(info);
		report.subgroup().appendEntry(newReport);

		if (sequence != nullptr)
		{
			// generate default constructor function if none is available
			if (not atom.hasMember("^new"))
				atom.renameChild("^default-new", "^new");
			return true;
		}
		atom.hasErrors = true;
		return false;
	}


	bool SequenceBuilder::instanciateAtomFunc(uint32_t& instanceid, Atom& funcAtom, uint32_t retlvid, uint32_t p1, uint32_t p2)
	{
		assert(funcAtom.isFunction());
		assert(frame != nullptr);
		auto& frameAtom = frame->atom;


		if (unlikely((p1 != 0 and not frame->verify(p1)) or (p2 != 0 and not frame->verify(p2))))
		{
			if (retlvid != 0)
				frame->invalidate(retlvid);
			return false;
		}

		overloadMatch.clear(); // reset
		if (p1 != 0) // first parameter
		{
			overloadMatch.input.params.indexed.emplace_back(frameAtom.atomid, p1);

			if (p2 != 0) // second parameter
				overloadMatch.input.params.indexed.emplace_back(frameAtom.atomid, p2);
		}

		if (retlvid != 0) // return value
			overloadMatch.input.rettype.push_back(CLID{frameAtom.atomid, retlvid});

		// disable any hint or error report for now, to avoid spurious messages
		// if there is no error.
		overloadMatch.canGenerateReport = false;

		TypeCheck::Match match = overloadMatch.validate(funcAtom);
		if (unlikely(TypeCheck::Match::none == match))
		{
			// fail - try again to produce error message, hint, and any suggestion
			auto err = (error() << "cannot call '" << funcAtom.keyword() << ' ');
			funcAtom.retrieveFullname(err.data().message);
			err << '\'';
			overloadMatch.canGenerateReport = true;
			overloadMatch.report = std::ref(err);
			overloadMatch.validate(funcAtom);
			if (retlvid != 0)
				frame->invalidate(retlvid);
			return false;
		}

		decltype(FuncOverloadMatch::result.params) params;
		params.swap(overloadMatch.result.params);

		// instanciate the called func
		Logs::Message::Ptr subreport;
		InstanciateData info{subreport, funcAtom, cdeftable, context, params};
		bool instok = doInstanciateAtomFunc(subreport, info, retlvid);
		instanceid = info.instanceid;

		if (unlikely(not instok and retlvid != 0))
			frame->invalidate(retlvid);
		return instok;
	}



	bool SequenceBuilder::doInstanciateAtomFunc(Logs::Message::Ptr& subreport, InstanciateData& info, uint32_t retlvid)
	{
		// even within a typeof, any new instanciation must see their code generated
		// (and its errors generated)
		info.canGenerateCode = true; // canGenerateCode();

		auto* sequence = InstanciateAtom(info);
		report.appendEntry(subreport);
		if (unlikely(nullptr == sequence))
		{
			success = false;
			return false;
		}

		if (unlikely(info.instanceid == static_cast<uint32_t>(-1)))
		{
			ICE(info.returnType, "return: invalid instance id");
			return false;
		}

		// import the return type of the instanciated sequence
		auto& spare = cdeftable.substitute(retlvid);
		spare.kind = info.returnType.kind;
		spare.atom = info.returnType.atom;
		if (not info.returnType.isVoid())
		{
			spare.instance = true; // force some values just in case
			if (unlikely(not spare.isBuiltinOrVoid() and spare.atom == nullptr))
			{
				ICE() << "return: invalid atom for return type";
				return false;
			}
		}
		return true;
	}



	namespace // anonymous
	{

		static inline bool PrepareSignatureFromResult(ClassdefTableView& cdeftable, Signature& signature,
			const std::vector<FuncOverloadMatch::ParamCall>& params)
		{
			uint32_t count = (uint32_t) params.size();
			signature.parameters.resize(count);

			for (uint32_t i = 0; i != count; ++i)
			{
				assert(params[i].cdef != nullptr);
				auto& cdef = *(params[i].cdef);
				auto& param = signature.parameters[i];

				param.atom = const_cast<Atom*>(cdeftable.findClassdefAtom(cdef));
				param.kind = cdef.kind;
				param.qualifiers = cdef.qualifiers;
			}
			return true;
		}

	} // anonymous namespace


	inline void SequenceBuilder::pushParametersFromSignature(LVID atomid, const Signature& signature)
	{
		assert(atomid != 0);
		// magic constant +2
		//  * +1: all clid are 1-based (0 is reserved for the atom itself, not for an internal var)
		//  * +1: the CLID{X, 1} is reserved for the return type

		// unused pseudo register
		CLID clid{atomid, 0};
		cdeftable.addSubstitute(nyt_void, nullptr, Qualifiers()); // unused, 1-based

		// redefine return type {atomid,1}
		clid.reclass(1);
		auto& rettype = cdeftable.rawclassdef(clid);
		assert(atomid == rettype.clid.atomid());
		Atom* atom = likely(not rettype.isBuiltinOrVoid()) ? cdeftable.findRawClassdefAtom(rettype) : nullptr;
		cdeftable.addSubstitute(rettype.kind, atom, rettype.qualifiers);

		uint32_t count = signature.parameters.size();
		for (uint32_t i = 0; i != count; ++i)
		{
			auto& param = signature.parameters[i];
			cdeftable.addSubstitute(param.kind, param.atom, param.qualifiers);
		}
	}


	IR::Sequence* InstanciateAtom(InstanciateData& info)
	{
		// prepare the matching signature
		Signature signature;
		if (not info.params.empty())
		{
			if (unlikely(not PrepareSignatureFromResult(info.cdeftable, signature, info.params)))
				return nullptr;
			assert(info.params.size() == signature.parameters.size());
		}

		// try to pick an existing instanciation
		{
			IR::Sequence* sequence = nullptr;
			uint32_t ix = info.atom.findInstance(sequence, signature);
			if (ix != static_cast<uint32_t>(-1))
			{
				info.returnType.import(signature.returnType);
				info.instanceid = ix;
				return sequence;
			}
		}

		// instanciate the function

		info.report = new Logs::Message(Logs::Level::none);
		Logs::Report report{*info.report};

		if (unlikely(info.atom.opcodes.sequence == nullptr))
		{
			report.ICE() << "imvalid null opcode sequence for atom ";
			return nullptr;
		}

		// the new sequence for the instanciated function
		auto out = std::make_unique<IR::Sequence>();
		{
			// the original IR sequence (generated from the AST)
			auto& sourceSequence = *(info.atom.opcodes.sequence);
			// new layer for the cdeftable
			ClassdefTableView newView{info.cdeftable, info.atom.atomid, signature.parameters.size()};

			// instanciate the sequence attached to the atom
			auto builder =
				std::make_unique<SequenceBuilder>(report.subgroup(), newView, info.context, *out, sourceSequence);

			builder->pushParametersFromSignature(info.atom.atomid, signature);
			if (info.parentAtom)
				builder->layerDepthLimit = 2; // allow the first one

			// instanciate the atom !
			bool success = builder->readAndInstanciate(info.atom.opcodes.offset);

			// post-process the output sequence to update the type of all
			// stack-allocated variables
			// (always update, easier for debugging)
			postProcessStackAlloc(*out, newView, info.atom.atomid);

			// keep the types deduced
			if (likely(success) and info.shouldMergeLayer)
				newView.mergeSubstitutes();


			// Generating the full name of the symbol
			// (example: "func A.foo(b: ref __i32): ref __i32")
			String symbolName;
			if (success or Config::Traces::printGeneratedOpcodeSequence)
			{
				symbolName << newView.keyword(info.atom) << ' ';
				info.atom.retrieveCaption(symbolName, newView);
			}

			if (Config::Traces::printGeneratedOpcodeSequence)
			{
				report.info();
				auto trace = report.subgroup();
				auto entry = trace.trace();
				entry.message.prefix << symbolName;

				Clob text;
				out.get()->print(text, &newView.atoms());
				text.replace("\n", "\n    ");
				text.trimRight();
				trace.trace() << "{\n    " << text << "\n}";
				trace.info(); // for beauty
				trace.info(); // for beauty
				trace.info(); // for beauty
			}

			// retrieving the return type
			if (success)
			{
				if (info.atom.isFunction())
				{
					auto& cdefReturn = newView.classdef(CLID{info.atom.atomid, 1});

					if (not cdefReturn.isBuiltinOrVoid())
					{
						auto* atom = newView.findClassdefAtom(cdefReturn);
						if (atom)
						{
							signature.returnType.mutateToAtom(atom);
							info.returnType.mutateToAtom(atom);
						}
						else
						{
							report.ICE() << "invalid atom pointer in func return type for '"
								<< info.atom.fullname() << '\'';
							success = false;
						}
					}
					else
					{
						signature.returnType.kind = cdefReturn.kind;
						info.returnType.kind = cdefReturn.kind;
					}
				}
				else
				{
					signature.returnType.mutateToVoid();
					info.returnType.mutateToVoid();
				}

				if (likely(success))
				{
					info.instanceid = info.atom.assignInstance(signature, out.get(), symbolName);
					return out.release();
				}
			}
		}

		if (debugmode)
			out->emitComment("error while compiling");
		info.instanceid = info.atom.assignInvalidInstance(signature);
		return nullptr;
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
