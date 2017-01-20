#include "intrinsic-table.h"

using namespace yuni;


namespace ny {


IntrinsicTable::IntrinsicTable() {
	m_intrinsics.reserve(64);
}


bool IntrinsicTable::add(const AnyString& name, void* callback, nytype_t ret, va_list argp) {
	if (YUNI_UNLIKELY(name.empty() or (0 == m_names.count(name))))
		return false;
	if (YUNI_UNLIKELY(nullptr == callback))
		return false;
	auto intrinsic = make_ref<Intrinsic>(name, callback);
	intrinsic->rettype = ret;
	intrinsic->id = (uint32_t) m_intrinsics.size();
	uint32_t count = 0;
	do {
		int i = va_arg(argp, int);
		if (i == 0)
			break;
		if (unlikely(count >= config::maxPushedParameters or i >= static_cast<int>(nyt_count)))
			return false;
		intrinsic->params[count] = static_cast<nytype_t>(i);
		++count;
	}
	while (true);
	intrinsic->paramcount = count;
	m_intrinsics.emplace_back(intrinsic);
	m_names.insert(std::make_pair(AnyString{intrinsic->name}, intrinsic));
	return true;
}


} // namespace ny
