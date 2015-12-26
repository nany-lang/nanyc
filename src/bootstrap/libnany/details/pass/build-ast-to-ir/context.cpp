
#include "context.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	void Context::useNamespace(const AnyString& nmspc)
	{
		if (not nmspc.empty())
		{
			nmspc.words(".", [&](const AnyString& part) -> bool
			{
				program.emitNamespace(part);
				return true;
			});
		}
	}


	void Context::generateLineIndexes(const AnyString& content)
	{
		uint line = 1;
		uint length = content.size();
		for (uint i = 0; i != length; ++i)
		{
			if (content[i] == '\n')
				offsetToLine.emplace(i, ++line);
		}
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
