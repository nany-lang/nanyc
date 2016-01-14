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


	YString Stacktrace<true>::dump(const AtomMap& map) const
	{
		YString out;

		uint32_t i = 0;
		for (auto* pointer = topframe; (pointer > baseframe); --pointer, ++i)
		{
			auto& frame = *pointer;
			out << "  at #" << i << ": ";
			out << map.fetchProgramCaption(frame.atomidInstance[0], frame.atomidInstance[1]);
			out << " (from '";

			auto* atom = map.findAtom(frame.atomidInstance[0]);
			if (atom)
			{
				out << atom->origin.filename;
				if (atom->origin.line != 0)
					out << ':' << atom->origin.line;
			}
			else
				out << "<invalid-atom>";

			out << "')\n";
		}
		return out;
	}






} // namespace VM
} // namespace Nany
