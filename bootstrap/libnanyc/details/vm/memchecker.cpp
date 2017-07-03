#include "memchecker.h"
#include "details/vm/console.h"

using namespace Yuni;


namespace ny {
namespace vm {


void MemChecker<true>::releaseAll() {
	if (not ownedPointers.empty()) {
		for (auto& pair : ownedPointers)
			free((void*) pair.first); // size: pair.second.objsize;
		ownedPointers.clear();
	}
}


void MemChecker<true>::printLeaks(const nyprogram_cf_t& cf) const noexcept {
	auto cerr = [&cf](const AnyString& string) {
		ny::vm::console::cerr(cf, string);
	};
	ShortString128 msg;
	msg << "\n\n=== nanyc vm: memory leaks detected in ";
	msg << ownedPointers.size() << " blocks ===\n";
	cerr(msg);
	try {
		for (auto& pair: ownedPointers) {
			msg.clear();
			msg << "    block " << (void*) pair.first << ' ';
			msg << pair.second.objsize << " bytes at ";
			msg << pair.second.origin;
			msg << '\n';
			cerr(msg);
		}
	}
	catch (...) {
		cerr("<received c++exception>");
	}
	cerr("\n");
}


} // namespace vm
} // namespace ny
