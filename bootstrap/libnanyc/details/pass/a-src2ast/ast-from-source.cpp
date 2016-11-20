#include "details/context/source.h"
#include "details/context/build-info.h"



namespace ny
{

	bool Source::passASTFromSourceWL()
	{
		auto& parser = pBuildInfo->parsing.parser;
		switch (m_type)
		{
			case Type::file:
				return parser.loadFromFile(m_filename) and parser.root;

			case Type::memory:
				return parser.load(m_content) and parser.root;
		}

		return false;
	}


} // namespace ny
