#include "memchecker.h"

using namespace Yuni;



namespace Nany
{
namespace VM
{


	void MemChecker<true>::printLeaks(const nyprogram_cf_t& cf) const
	{
		auto print = [&](const AnyString& msg) {
			cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
		};

		String msg;
		print("\n\n=== nany vm: memory leaks detected in ");
		msg << ownedPointers.size();
		print(msg);
		print(" blocks ===\n");

		for (auto& pair: ownedPointers)
		{
			msg.clear();
			msg << "    block " << (void*) pair.first << ' ';
			msg << pair.second.objsize << " bytes at ";
			msg << pair.second.origin;
			msg << '\n';
			print(msg);
		}
		print("\n");
	}






} // namespace VM
} // namespace Nany
