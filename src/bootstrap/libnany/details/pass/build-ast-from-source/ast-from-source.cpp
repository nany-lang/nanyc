#include "details/context/source.h"
#include "details/context/build-info.h"



namespace Nany
{

	bool Source::passASTFromSourceWL()
	{
		auto& parser = pBuildInfo->parsing.parser;
		switch (pType)
		{
			case Type::file:
				return parser.loadFromFile(pFilename) and parser.root;

			case Type::memory:
				return parser.load(pContent) and parser.root;
		}

		return false;
	}




} // namespace Nany
