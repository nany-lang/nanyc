#include "memchecker.h"

using namespace Yuni;



namespace Nany
{
namespace VM
{


	void MemChecker<true>::printLeaks(nycontext_t& context) const
	{
		String msg;
		msg.reserve(64 + (uint32_t) ownedPointers.size() * 36); // arbitrary

		msg << "\n\n=== nany vm: memory leaks detected in ";
		msg << ownedPointers.size() << " blocks ===\n";

		for (auto& pair: ownedPointers)
		{
			msg << "    block " << (void*) pair.first << ' ';
			msg << pair.second.objsize << " bytes at ";
			msg << pair.second.origin;
			msg << '\n';
		}
		msg << '\n';
		context.console.write_stderr(&context, msg.c_str(), msg.size());
	}






} // namespace VM
} // namespace Nany
