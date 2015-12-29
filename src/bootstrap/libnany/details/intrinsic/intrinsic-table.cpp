#include "intrinsic-table.h"



namespace Nany
{

	IntrinsicTable::IntrinsicTable()
	{
		pIntrinsics.reserve(64);
	}


	bool IntrinsicTable::add(const AnyString& name, uint32_t flags, void* callback, nytype_t ret, va_list argp)
	{
		if (YUNI_UNLIKELY(name.empty() or (0 == pByNames.count(name))))
			return false;
		if (YUNI_UNLIKELY(nullptr == callback))
			return false;

		auto* intrinsic = new Intrinsic(name, callback);
		intrinsic->rettype = ret;
		intrinsic->flags   = flags;

		uint32_t count = 0;
		do
		{
			int i = va_arg(argp, int);
			if (i == 0)
				break;

			if (count >= Config::maxPushedParameters or i >= static_cast<int>(nyt_count))
			{
				delete intrinsic;
				return false;
			}

			intrinsic->params[count] = static_cast<nytype_t>(i);
			++count;
		}
		while(true);

		intrinsic->paramcount = count;


		pIntrinsics.emplace_back(intrinsic);
		pByNames.insert(std::make_pair(AnyString{intrinsic->name}, std::ref(*(intrinsic))));
		return true;
	}




} // namespace Nany
