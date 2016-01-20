#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::generateMemberVarDefaultClone()
	{
		assert(canGenerateCode());
		assert(lastOpcodeStacksizeOffset != (uint32_t) -1);
		assert(not atomStack.empty());

		// special location: in a constructor - initializing all variables with their def value
		// note: do not keep a reference on 'out.at...', since the internal buffer might be reized
		auto& frame = atomStack.back();
		auto& parentAtom = *(frame.atom.parent);

		// do not warn about rhs (unused). The parameter is used, but not via its name
		// reminder: 1-based, 1: returntype, 2: self, 3: first parameter
		assert(3 < frame.lvids.size());
		frame.lvids[3].warning.unused = false;


		std::vector<std::reference_wrapper<Atom>> atomvars;
		Atom* userDefinedClone = nullptr;

		parentAtom.eachChild([&](Atom& subatom) -> bool
		{
			if (subatom.isMemberVariable())
			{
				atomvars.emplace_back(std::ref(subatom));
			}
			else
			{
				if (subatom.isOperator() and subatom.name == "^clone")
					userDefinedClone = &subatom;
			}
			return true; // next
		});
		if (atomvars.empty() and userDefinedClone == nullptr)
			return;

		// create new local variables for performing the cline
		uint32_t more = (uint32_t)atomvars.size() * 2 + (userDefinedClone ? 1 : 0);
		uint32_t lvid = createLocalVariables(more);

		for (auto& subatomref: atomvars)
		{
			auto& subatom = subatomref.get();
			auto& cdef    = cdeftable.classdef(subatom.returnType.clid); // type of the var member
			// out.emitComment(String{"clone for '"} << subatom.name << '\'');

			if (not cdef.isBuiltinOrVoid())
			{
				// read the pointer rhs
				uint32_t rhsptr = lvid++;
				uint32_t lhsptr = lvid++;
				out.emitFieldget(rhsptr, /*rhs*/ 3, subatom.varinfo.effectiveFieldIndex);

				auto& origin  = frame.lvids[rhsptr].origin.varMember;
				origin.self   = 2;
				origin.atomid = subatom.atomid;
				origin.field  = subatom.varinfo.effectiveFieldIndex;

				auto& cdeflhs = cdeftable.substitute(lhsptr);
				cdeflhs.import(cdef);
				cdeflhs.qualifiers = cdef.qualifiers; // qualifiers must be preserved

				auto& cdefrhs = cdeftable.substitute(rhsptr);
				cdefrhs.import(cdef);
				cdefrhs.qualifiers = cdef.qualifiers;

				instanciateAssignment(frame, lhsptr, rhsptr, false);
				out.emitFieldset(lhsptr, /*self*/ 2, subatom.varinfo.effectiveFieldIndex);
			}
			else
			{
				out.emitFieldget(lvid,  /*rhs*/  3, subatom.varinfo.effectiveFieldIndex);
				out.emitFieldset(lvid, /*self*/ 2, subatom.varinfo.effectiveFieldIndex);
				++lvid;
			}
		}

		// call to user-defined operator dispose
		if (userDefinedClone)
		{
			out.emitPush(2); // self
			out.emitPush(3); // rhs
			out.emitCall(lvid, userDefinedClone->atomid, 0);
			++lvid;
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
