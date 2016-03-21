#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::generateMemberVarDefaultInitialization()
	{
		assert(frame != nullptr);
		assert(canGenerateCode());
		assert(lastOpcodeStacksizeOffset != (uint32_t) -1);

		// special location: in a constructor - initializing all variables with their def value
		// note: do not keep a reference on 'out.at...', since the internal buffer might be reized
		auto& funcAtom = frame->atom;
		if (unlikely(funcAtom.parent == nullptr))
			return (void)(ICE() << "invalid parent atom for variable initialization in ctor");

		auto& parentAtom = *(funcAtom.parent);
		if (parentAtom.empty())
			return;

		std::vector<std::reference_wrapper<Atom>> atomvars;
		atomvars.reserve(parentAtom.size());

		parentAtom.eachChild([&](Atom& subatom) -> bool
		{
			if (subatom.isMemberVarDefaultInit())
				atomvars.emplace_back(std::ref(subatom));
			return true; // next
		});
		if (atomvars.empty())
			return;


		// more variables for calling init funcs
		uint32_t lvid = createLocalVariables((uint32_t) atomvars.size());

		// there are two solutions for initializing a variable member:
		// - the default value
		// - from a 'self' parameter (which may vary from a ctor to another)
		//
		// In both cases, the default value (thus the func for initializing the var)
		// must be valid (thus instanciated for being checked)

		if (frame->selfParameters.get() == nullptr)
		{
			for (auto& subatomref: atomvars)
			{
				auto& subatom = subatomref.get();
				if (debugmode)
					out.emitComment(String() << "initialization for " << subatom.name << " via default-init");

				uint32_t instanceid = static_cast<uint32_t>(-1);
				bool localSuccess = instanciateAtomFunc(instanceid, subatom, /*ret*/0, /*self*/2);

				if (likely(localSuccess))
				{
					out.emitPush(2); // %2 -> self
					out.emitCall(lvid, subatom.atomid, instanceid);
				}
				++lvid; // always increment it to ease debugging
			}
		}
		else
		{
			auto& selfParameters = *(frame->selfParameters.get());
			auto selfparamlistEnd = selfParameters.end();
			if (debugmode)
				out.emitComment(String() << "initialization with " << selfParameters.size() << " self-parameter(s)");

			for (auto& subatomref: atomvars)
			{
				auto& subatom = subatomref.get();

				// extracting varname from ^default-var-%42-varname
				auto endOffset = subatom.name.find_last_of('-');
				if (unlikely(not (endOffset < subatom.name.size())))
					return (void)(ICE() << "invalid string offset");
				AnyString varname{subatom.name, endOffset + 1};

				uint32_t instanceid = (uint32_t) -1;
				bool localSuccess = instanciateAtomFunc(instanceid, subatom, /*ret*/0, /*self*/2);

				auto selfIT = selfParameters.find(varname);
				if (selfIT == selfparamlistEnd)
				{
					if (debugmode)
						out.emitComment(String() << "initialization for " << subatom.name << " via default-init");

					if (localSuccess)
					{
						out.emitPush(2); // %2 -> self
						out.emitCall(lvid, subatom.atomid, instanceid);
					}
				}
				else
				{
					if (debugmode)
						out.emitComment(String() << "initialization for " << subatom.name << " via self-parameter");

					if (localSuccess)
					{
						// lvid of the parameter value
						uint32_t paramlvid = selfIT->second.first;

						// set the type of 'lvid' to allow assignment
						auto& cdef  = cdeftable.classdef(CLID{frame->atomid, paramlvid});
						auto& spare = cdeftable.substitute(lvid);
						spare.import(cdef);

						instanciateAssignment(*frame, lvid, paramlvid, false);

						// retrieving real atom from name to get the field index
						const Atom* varatom = nullptr;
						parentAtom.eachChild(varname, [&](const Atom& atom) -> bool
						{
							varatom = &atom;
							return false;
						});
						if (unlikely(!varatom))
							return (void)(ICE() << "invalid atom for self init " << varname);

						out.emitFieldset(lvid, /*self*/ 2, varatom->varinfo.effectiveFieldIndex);

						// acquire 'lvid' to keep it alive
						if (canBeAcquired(cdef))
							out.emitRef(lvid);
					}

					// mark it as 'used' to suppress spurious error reporting
					selfIT->second.second = true;
				}

				++lvid; // always increment it to ease debugging
			}

			// checking for non used self parameters
			// if any error arise, it is not critical (from the type checking's point of view)
			for (auto& pair: selfParameters)
			{
				if (unlikely(not pair.second.second))
					error() << "unknown member '" << pair.first << "' for automatic variable assignment";
			}

			// release memory
			frame->selfParameters.reset();
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
