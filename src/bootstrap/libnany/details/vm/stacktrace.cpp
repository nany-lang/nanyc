#include "stacktrace.h"

using namespace Yuni;




namespace Nany
{
namespace VM
{

	void Stacktrace<true>::grow()
	{
		auto offbase     = reinterpret_cast<std::uintptr_t>(baseframe);
		auto offtop      = reinterpret_cast<std::uintptr_t>(topframe);
		auto size        = (offtop - offbase) / sizeof(Frame);
		auto newcapacity = size + 64;

		baseframe  = (Frame*)::realloc(baseframe, sizeof(Frame) * newcapacity);
		if (YUNI_UNLIKELY(!baseframe))
			throw std::bad_alloc();
		topframe   = baseframe + size;
		upperLimit = baseframe + newcapacity;
	}


	void Stacktrace<true>::dump(nycontext_t& ctx, const AtomMap& map) const
	{
		// this routine does not allocate memory to handle extreme situations

		ShortString128 tmp;
		uint32_t i = 0;

		for (auto* pointer = topframe; (pointer > baseframe); --pointer, ++i)
		{
			auto& frame = *pointer;

			ctx.console.write_stderr(&ctx, "    at #", 8);
			tmp.clear();
			tmp << i << ": ";
			ctx.console.write_stderr(&ctx, tmp.c_str(), tmp.size());
			const auto& caption = map.fetchProgramCaption(frame.atomidInstance[0], frame.atomidInstance[1]);
			ctx.console.write_stderr(&ctx, caption.c_str(), caption.size());

			ctx.console.write_stderr(&ctx, "\n       '", 9);

			auto* atom = map.findAtom(frame.atomidInstance[0]);
			if (atom)
			{
				ctx.console.write_stderr(&ctx, atom->origin.filename.c_str(), atom->origin.filename.size());
				if (atom->origin.line != 0)
				{
					tmp.clear();
					tmp << ':' << atom->origin.line;
					ctx.console.write_stderr(&ctx, tmp.c_str(), tmp.size());
				}
			}
			else
				ctx.console.write_stderr(&ctx, "<invalid-atom>", 14);

			ctx.console.write_stderr(&ctx, "'\n", 2);
		}
	}






} // namespace VM
} // namespace Nany
