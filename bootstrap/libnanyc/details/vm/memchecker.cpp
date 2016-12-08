#include "memchecker.h"
#include "details/vm/console.h"

using namespace Yuni;


namespace ny {
namespace vm {


void MemChecker<true>::releaseAll(nyallocator_t& allocator) {
	if (not ownedPointers.empty()) {
		for (auto& pair : ownedPointers)
			allocator.deallocate(&allocator, (void*) pair.first, pair.second.objsize);
		ownedPointers.clear();
	}
}


void MemChecker<true>::printLeaks(const nyprogram_cf_t& cf) const {
	auto cerr = [&cf](const AnyString& string) {
		ny::vm::console::cerr(cf, string);
	};
	ShortString128 msg;
	msg << "\n\n=== nanyc vm: memory leaks detected in ";
	msg << ownedPointers.size() << " blocks ===\n";
	cerr(msg);
	for (auto& pair: ownedPointers) {
		msg.clear();
		msg << "    block " << (void*) pair.first << ' ';
		msg << pair.second.objsize << " bytes at ";
		msg << pair.second.origin;
		msg << '\n';
		cerr(msg);
	}
	cerr("\n");
}


} // namespace vm
} // namespace ny
