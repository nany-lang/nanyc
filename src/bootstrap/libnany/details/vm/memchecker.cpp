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
		std::unordered_map<uint64_t, uint64_t> loses;
		for (auto& pair: ownedPointers)
			loses[pair.second]++;

		for (auto& pair: loses)
		{
			msg << "    block " << pair.first << " bytes * " << pair.second;
			msg << " = ";
			msg << (pair.second * pair.first);
			msg << " bytes\n";
		}

		msg << '\n';
		context.console.write_stderr(&context, msg.c_str(), msg.size());
	}






} // namespace VM
} // namespace Nany
