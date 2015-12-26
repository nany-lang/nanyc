#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void ProgramBuilder::generateMemberVarDefaultDispose()
	{
		assert(canGenerateCode());
		assert(lastOpcodeStacksizeOffset != (uint32_t) -1);

		// special location: in a constructor - initializing all variables with their def value
		// note: do not keep a reference on 'out.at...', since the internal buffer might be reized
		uint32_t lvid = out.at<IR::ISA::Op::stacksize>(lastOpcodeStacksizeOffset).add;
		auto& parentAtom = *(atomStack.back().atom.parent);


		std::vector<std::reference_wrapper<Atom>> atomvars;
		Atom* userDefinedDispose = nullptr;

		parentAtom.eachChild([&](Atom& subatom) -> bool
		{
			if (subatom.isMemberVariable())
			{
				atomvars.emplace_back(std::ref(subatom));
			}
			else
			{
				if (subatom.isOperator() and subatom.name == "^dispose")
					userDefinedDispose = &subatom;
			}
			return true; // next
		});
		if (atomvars.empty() and userDefinedDispose == nullptr)
			return;

		// resize
		uint32_t lvidinc = (uint32_t) atomvars.size();
		if (userDefinedDispose)
			++lvidinc;

		auto& frame = atomStack.back();
		frame.resizeRegisterCount(lvid + (uint32_t)atomvars.size() + lvidinc + 1, cdeftable);

		// call to user-defined operator dispose
		if (userDefinedDispose)
		{
			out.emitStackalloc(lvid, nyt_any);
			out.emitPush(2); // self
			out.emitCall(lvid, userDefinedDispose->atomid, 0);
			++lvid;
		}

		// ... then release all variables !
		for (auto& subatomref: atomvars)
		{
			auto& subatom = subatomref.get();
			auto& cdef    = cdeftable.classdef(subatom.returnType.clid);

			if (not cdef.isBuiltinOrVoid())
			{
				// temporary variable to force increment enven if an error has occured
				// (to avoid for looking for spurious bugs while debugging)
				uint32_t reglvid = lvid++;
				// type propagation
				cdeftable.substitute(reglvid).import(cdef);

				// out.emitComment(String() << "dispose for " << subatom.name);
				// read the pointer
				out.emitFieldget(reglvid, /*self*/ 2, subatom.varinfo.effectiveFieldIndex);

				auto& origin  = frame.lvids[reglvid].origin.varMember;
				origin.self   = 2;
				origin.atomid = subatom.atomid;
				origin.field  = subatom.varinfo.effectiveFieldIndex;

				auto* typeAtom = cdeftable.findClassdefAtom(cdef);
				if (unlikely(nullptr == typeAtom))
				{
					auto ice = (ICE() << "invalid atom from " << cdef.clid);
					ice << " for disposing member variable '";
					subatom.printFullname(ice.data().message, false);
					ice << "'";
					continue;
				}

				if (0 == typeAtom->classinfo.dtor.atomid)
				{
					if (not instanciateAtomClassDestructor(*typeAtom, reglvid))
						continue;
				}

				auto& classinfo = typeAtom->classinfo;
				out.emitUnref(reglvid, classinfo.dtor.atomid, classinfo.dtor.instanceid);
			}
			else
			{
				// out.emitComment(String() << "builtin dispose for " << subatom.name);
			}
		}

		// updating the stack size
		out.at<IR::ISA::Op::stacksize>(lastOpcodeStacksizeOffset).add = lvid;
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
