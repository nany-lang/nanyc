#include "check-for-valid-identifier-name.h"
#include "details/ast/ast.h"
#include "details/reporting/report.h"
#include "details/fwd.h"
#include <yuni/io/file.h>
#include <unordered_set>
#include <unordered_map>

using namespace Yuni;





namespace Nany
{

	static const std::unordered_set<AnyString> reservedKeywords =
	{
		"assert", "self", "override", "new", "is",
		"func", "class", "typedef",
		"for", "while", "do", "in", "each", "if", "then", "else", "switch",
		"return", "raise",
		"const", "var", "ref", "cref",
		"and", "or", "mod", "xor", "not",
		// boolean types
		"true", "false",
		"i8", "i6", "i32", "i64", "i128", "i256", "i512",
		"u8", "u16", "u32", "u64", "u128", "u256", "u512",
		"f32", "f64",
		"bool", "string", "pointer",
		// null keyword
		"null", "void"
	};

	static const std::unordered_set<AnyString> operatorKeywords =
	{
		"new", "dispose",
		"and", "or", "mod", "xor", "not",
		"+", "-", "*", "/", "^", "--", "++"
		"<<", ">>", "!=", "==", "<", ">", "<=", ">=",
		// "=", operator copy should be used instead
		"+=", "-=", "*=", "/=", "^=",
		"()", "[]"
	};

	static const std::unordered_map<AnyString, AnyString> opnormalize =
	{
		{ "and", "^and" },
		{ "or",  "^or"  },
		{ "mod", "^mod" },
		{ "xor", "^xor" },
		{ "not", "^not" },
		{ "+",   "^+"   }, { "++",  "^++"  },
		{ "-",   "^-"   }, { "--",  "^--"  },
		{ "*",   "^*"   }, { "/",   "^/"   },
		{ "^",   "^^"   },
		{ "<<",  "^<<"  }, { ">>",  "^>>"  },
		{ "<=",  "^<="  }, { ">=",  "^>="  },
		{ "<",   "^<"   }, { ">",   "^>"   },
		{ "==",  "^=="  }, { "!=",  "^!="  },
		{ "+=",  "^+="  }, { "-=",  "^-="  },
		{ "*=",  "^*="  }, { "/=",  "^/="  },
		{ "^=",  "^^="  },
		{ "()",  "^()"  }, { "[]",  "^[]"  }
	};







	AnyString normalizeOperatorName(AnyString name)
	{
		// to deal with grammar's potential glitches when eating tokens
		// (it would considerably slow down the parsing to improve it)
		name.trimRight();

		if (YUNI_UNLIKELY(name.empty())) // bug within the AST parser for + -
			return "^+";

		auto tit = opnormalize.find(name);
		if (YUNI_LIKELY(tit != opnormalize.end()))
			return tit->second;

		assert(false and "unknown identifier for normalization");
		return name;
	}


	bool checkForValidIdentifierName(Logs::Report& report, const Nany::Node& node, const AnyString& name, bool isOperator)
	{
		// do never accept empty names
		if (unlikely(name.empty()))
		{
			report.error() << "invalid empty name";
			return false;
		}

		// the name is always valid if the node comes from internal AST transformations
		if (node.metadata and AST::metadata(node).fromASTTransformation)
			return true;

		if (YUNI_LIKELY(not isOperator))
		{
			// names with '_' as prefix are for internal uses only
			if (unlikely(name.first() == '_'))
			{
				auto err = report.error() << "invalid identifier name '" << name << "': underscore as a prefix is not allowed";
				err.message.origins.location.pos.offsetEnd = err.message.origins.location.pos.offset + name.size();
				return false;
			}

			// warning - ascii only chars
			if (false)
			{
				auto end = name.utf8end();
				for (auto i = name.utf8begin(); i != end; ++i)
				{
					if (not i->isAscii())
					{
						auto wrn = report.warning() << "invalid identifier name: the name contains non-ascii characters";
						wrn.message.origins.location.pos.offsetEnd = wrn.message.origins.location.pos.offset + name.size();
						break;
					}
				}
			}

			// checking if the id name is not from one of reserved keywords
			if (unlikely(reservedKeywords.count(name) != 0))
			{
				auto err = report.error() << "'" << name << "' is a reserved keyword";
				err.message.origins.location.pos.offsetEnd = err.message.origins.location.pos.offset + name.size();
				return false;
			}
		}
		else
		{
			// checking if the operator name is one from the list
			if (unlikely(operatorKeywords.count(name) == 0)) // not found
			{
				auto err = report.error() << "invalid identifier name: '" << name << "' is not a valid operator";
				err.message.origins.location.pos.offsetEnd = err.message.origins.location.pos.offset + name.size();
				return false;
			}
		}


		if (unlikely(name.size() > 127u))
		{
			auto err = report.error() << "identifier name too long";
			err.message.origins.location.pos.offsetEnd = err.message.origins.location.pos.offset + name.size();
			return false;
		}
		return true;
	}






} // namespace Nany

