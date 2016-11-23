#pragma once
#include "interface.h"


namespace ny {


inline bool ClassdefInterface::hasSelf() const {
	return not (!pSelf);
}


inline Funcdef& ClassdefInterface::self() {
	assert(hasSelf());
	return *pSelf;
}


inline const Funcdef& ClassdefInterface::self() const {
	assert(hasSelf());
	return *pSelf;
}


inline bool ClassdefInterface::empty() const {
	return pInterface.empty();
}


inline void ClassdefInterface::clear() {
	pInterface.clear();
	pInterface.shrink_to_fit();
}


inline void ClassdefInterface::add(Funcdef* funcdef) {
	assert(funcdef != nullptr);
	pInterface.push_back(funcdef);
}


template<class C>
inline void ClassdefInterface::eachUnresolved(const C& callback) {
	for (auto& ptr : pInterface) {
		auto& funcdef = *ptr;
		if (not funcdef.isPartiallyResolved())
			callback(funcdef);
	}
}


} // namespace ny
