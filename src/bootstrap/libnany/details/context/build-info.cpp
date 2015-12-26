#include "build-info.h"



namespace Nany
{

	size_t BuildInfoSource::inspectMemoryUsage() const
	{
		size_t bytes = sizeof(BuildInfoSource);
		bytes += parsing.nmspc.first.capacity();
		bytes += parsing.program.sizeInBytes();
		return bytes;
	}




} // namespace Nany
