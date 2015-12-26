#include <yuni/yuni.h>
#include <yuni/string.h>
#include "nany/nany.h"
#include "details/grammar/nany.h"
#include <vector>
#include <functional>

using namespace Yuni;




namespace // anonymous
{

	static inline bool walkThroughAllNodes(const Nany::Node& allnodes)
	{
		std::vector<std::reference_wrapper<const Nany::Node>> stack;
		stack.reserve(128); // to reduce memory reallocations
		stack.push_back(std::cref(allnodes));

		do
		{
			auto& node = stack.back().get();
			if (Nany::ASTRuleIsError(node.rule))
				return false;

			stack.pop_back();
			for (auto it = node.children.rbegin(); it != node.children.rend(); ++it)
				stack.push_back(std::cref(*(*it)));
		}
		while (not stack.empty());
		return true;
	}


} // anonymous namespace




extern "C" nybool_t nany_utility_validator_check_file_n(const char* filename, size_t length)
{
	Nany::Parser parser;
	AnyString path{filename, (uint32_t)length};
	bool success = parser.loadFromFile(path);
	bool ret = (success and parser.root and walkThroughAllNodes(*(parser.root)));
	return ret ? nytrue : nyfalse;
}
