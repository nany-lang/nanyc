#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	bool SequenceBuilder::tryToCaptureVariable(const AnyString& name)
	{
		auto& currentAtom = frame->atom;
		auto clid = ([&](SequenceBuilder* sb) -> CLID
		{
			// try to find the name of the variable in the current instanciation stack
			// Obviously, the instanciation stack may not completely match the
			// user code's structure. That's why the atom where the variable is found
			// must be an ancestor of the current atom (via `findParent`, which
			// represents the user code's structure)

			// Try the current sequence builder, omitting the current frame
			for (auto* f = sb->frame->previous; f; f = f->previous)
			{
				uint32_t ix = f->findLocalVariable(name);
				if (ix != (uint32_t) -1 and currentAtom.findParent(f->atom))
					return CLID{f->atomid, ix};
			}

			// trying previous sequence builder
			for (sb = sb->parent; sb; sb = sb->parent)
			{
				for (auto* f = sb->frame; f; f = f->previous)
				{
					uint32_t ix = f->findLocalVariable(name);
					if (ix != (uint32_t) -1 and currentAtom.findParent(f->atom))
						return CLID{f->atomid, ix};
				}
			}
			return CLID{};
		} )(this);

		if (not clid.isVoid())
		{
			if (debugmode)
				warning() << "missing implementation for capturing the variable '" << name << '\'';
		}
		return false;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
