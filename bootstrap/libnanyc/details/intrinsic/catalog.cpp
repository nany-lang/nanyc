#include "catalog.h"

namespace ny {
namespace intrinsic {

Catalog::Catalog() {
	m_intrinsics.reserve(128);
}

Intrinsic& Catalog::makeIntrinsic(const AnyString& name, void* callback) {
	uint32_t id = size();
	m_intrinsics.emplace_back(callback, id);
	auto& intrinsic = m_intrinsics.back();
	m_names.emplace(std::piecewise_construct,
		std::forward_as_tuple(name), std::forward_as_tuple(id));
	return intrinsic;
}

} // namespace intrinsic
} // namespace ny
