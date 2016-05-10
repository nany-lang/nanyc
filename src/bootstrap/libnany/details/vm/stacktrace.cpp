#include "stacktrace.h"
#include "details/context/build.h"

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


	void Stacktrace<true>::dump(Build& build, const AtomMap& map) const
	{
		// this routine does not allocate memory to handle extreme situations

		build.printStderr("stack trace:\n");
		uint32_t i = 0;
		uint32_t count = 0;

		for (auto* pointer = topframe; (pointer > baseframe); --pointer, ++i)
			++count;

		i = 0;
		ShortString32 tmp;

		for (auto* pointer = topframe; (pointer > baseframe); --pointer, ++i)
		{
			auto& frame = *pointer;

			build.printStderr("    ");

			build.cerrColor(nyc_lightblue);
			tmp.clear() << '#' << i;
			build.printStderr(tmp);
			build.cerrColor(nyc_none);

			build.printStderr(" in '");

			const auto& caption = map.fetchSequenceCaption(frame.atomidInstance[0], frame.atomidInstance[1]);
			build.cerrColor(nyc_white);
			build.printStderr(caption);
			build.cerrColor(nyc_none);

			build.printStderr("' at '");

			auto* atom = map.findAtom(frame.atomidInstance[0]);
			if (atom)
			{
				build.printStderr(atom->origin.filename);
				if (atom->origin.line != 0)
				{
					tmp.clear();
					tmp << ':' << atom->origin.line;
					build.printStderr(tmp);
				}
			}
			else
				build.printStderr("<invalid-atom>");

			build.printStderr("'\n");
		}
	}






} // namespace VM
} // namespace Nany
