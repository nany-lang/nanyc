#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <vector>
#include "details/atom/atom-map.h"




namespace Nany
{
namespace VM
{
namespace Stack
{

	struct Frame
	{
		uint32_t atomid;
		uint32_t instanceid;
	};



	struct List final
	{
	public:
		List()
		{
			frames.reserve(32);
		}
		List(const List&) = default;

		void push(uint32_t atomid, uint32_t instanceid)
		{
			frames.emplace_back(Frame{.atomid = atomid, .instanceid = instanceid});
		}

		void pop()
		{
			frames.pop_back();
		}


		YString dump(const AtomMap& map) const
		{
			YString out;

			if (not frames.empty())
			{
				size_t i = frames.size();
				while (i-- != 0)
				{
					auto& frame = frames[i];
					out << "  at #" << i << ": ";
					out << map.fetchProgramCaption(frame.atomid, frame.instanceid);
					out << " (";

					auto* atom = map.findAtom(frame.atomid);
					if (atom)
					{
						out << atom->origin.filename;
						if (atom->origin.line != 0)
							out << ':' << atom->origin.line;
					}
					else
						out << "<invalid-atom>";

					out << ")\n";
				}
			}
			return out;
		}


	private:
		std::vector<Frame> frames;
	};





} // namespace Stack
} // namespace VM
} // namespace Nany
