#include <yuni/yuni.h>
#include <yuni/string.h>
#include "nany/nany.h"
#include "details/grammar/nany.h"
#include <vector>
#include <functional>

using namespace Yuni;




namespace // anonymous
{

	static inline bool tryFindErrorNode(const ny::AST::Node& allnodes)
	{
		std::vector<std::reference_wrapper<const ny::AST::Node>> stack;
		stack.reserve(128); // to reduce memory reallocations
		stack.push_back(std::cref(allnodes));
		do
		{
			auto& node = stack.back().get();
			if (ny::AST::ruleIsError(node.rule))
				return false;

			stack.pop_back();
			uint32_t i = node.children.size();
			while (i-- > 0)
				stack.push_back(std::cref(node.children[i]));
		}
		while (not stack.empty());
		return true;
	}

} // anonymous namespace




extern "C" nybool_t nytry_parse_file_n(const char* const filename, size_t length)
{
	bool success = false;
	if (length != 0 and length < 16*1024 and filename != nullptr)
	{
		try
		{
			String path{filename, static_cast<uint32_t>(length)};

			ny::AST::Parser parser;
			success = parser.loadFromFile(path);

			if (success)
				success = (parser.root != nullptr) ? tryFindErrorNode(*(parser.root)) : true; // empty AST
		}
		catch (...) {}
	}
	return success ? nytrue : nyfalse;
}


extern "C" nybool_t nytry_parse_file(const char* const filename)
{
	size_t length = (filename ? strlen(filename) : 0u);
	return nytry_parse_file_n(filename, length);
}
