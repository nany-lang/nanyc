#include "build-info.h"



namespace ny
{

	size_t BuildInfoSource::inspectMemoryUsage() const
	{
		size_t bytes = sizeof(BuildInfoSource);
		bytes += parsing.nmspc.first.capacity();
		bytes += parsing.sequence.sizeInBytes();
		return bytes;
	}




} // namespace ny
