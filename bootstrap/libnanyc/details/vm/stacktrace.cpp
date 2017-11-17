#include "stacktrace.h"

using namespace Yuni;

namespace ny::vm {

Stacktrace<true>::Stacktrace() {
	uint32_t capacity = 64;
	baseframe  = (Frame*)::malloc(sizeof(Frame) * capacity);
	if (unlikely(!baseframe))
		throw std::bad_alloc();
	upperLimit = baseframe + capacity;
	topframe   = baseframe;
}

Stacktrace<true>::~Stacktrace() {
	::free(baseframe);
}

void Stacktrace<true>::grow() {
	auto offbase     = reinterpret_cast<std::uintptr_t>(baseframe);
	auto offtop      = reinterpret_cast<std::uintptr_t>(topframe);
	auto size        = (offtop - offbase) / sizeof(Frame);
	auto newcapacity = size + 64;
	baseframe  = (Frame*)::realloc(baseframe, sizeof(Frame) * newcapacity);
	if (unlikely(!baseframe))
		throw std::bad_alloc();
	topframe   = baseframe + size;
	upperLimit = baseframe + newcapacity;
}

/*
void Stacktrace<true>::dump(const nyprogram_cf_t& cf, const AtomMap& map) const noexcept {
	auto cerr = [&cf](const AnyString& string) {
		ny::vm::console::cerr(cf, string);
	};
	auto color = [&cf](nyoldcolor_t cl) {
		ny::vm::console::color(cf, nycerr, cl);
	};
	color(nycold_none);
	cerr("\nstack trace:\n");
	uint32_t i = 0;
	ShortString64 tmp;
	for (auto* pointer = topframe; (pointer > baseframe); --pointer, ++i) {
		auto& frame = *pointer;
		try {
			cerr("    ");
			color(nycold_lightblue);
			cerr(tmp.clear() << '#' << i);
			color(nycold_none);
			cerr(" in '");
			auto caption = map.symbolname(frame.atomidInstance[0], frame.atomidInstance[1]);
			color(nycold_white);
			cerr(caption);
			color(nycold_none);
			cerr("' at '");
			auto atom = map.findAtom(frame.atomidInstance[0]);
			if (!!atom) {
				cerr(atom->origin.filename);
				if (atom->origin.line != 0)
					cerr(tmp.clear() << ':' << atom->origin.line);
			}
			else
				cerr("<invalid-atom>");
		}
		catch (...) {
			cerr("<received c++exception>");
		}
		cerr("'\n");
	}
}
*/

} // ny::vm
