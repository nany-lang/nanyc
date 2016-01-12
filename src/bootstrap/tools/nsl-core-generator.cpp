#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/io/file.h>
#include <iostream>

using namespace Yuni;





static void craftClassInt(Clob& o, uint32_t bits, bool issigned, const AnyString& license, const AnyString& filename)
{
	o.clear();
	char c = (issigned) ? 'i' : 'u';
	ShortString16 suffix;
	suffix << c << bits;

	auto craftOperator = [&](auto callback) {
		callback(bits, c);
		if (issigned)
			callback(bits / 2, 'u');
		o << '\n';
	};

	auto craft = [&](const AnyString& op, const AnyString& builtin, auto callback)
	{
		char sign = issigned ? 'i' : 'u';
		for (uint32_t b = bits; b >= 8; b /= 2)
		{
			callback(op, builtin, sign, b, "cref ", "cref ");
			callback(op, builtin, sign, b, "cref ", "__");
			callback(op, builtin, sign, b, "__", "cref ");
			callback(op, builtin, sign, b, "__", "__");
		}

		if (issigned)
		{
			sign = 'u';
			for (uint32_t b = bits / 2; b >= 8; b /= 2)
			{
				callback(op, builtin, sign, b, "cref ", "cref ");
				callback(op, builtin, sign, b, "cref ", "__");
				callback(op, builtin, sign, b, "__", "cref ");
				callback(op, builtin, sign, b, "__", "__");
			}
		}
		o << '\n';
	};


	o << license;
	o << "/// \\file    " << suffix << ".ny\n";
	o << "/// \\brief   Implementation of the class " << suffix << ", ";
	o << (issigned ? "Signed" : "Unsigned") << " integer with width of exactly " << bits << " bits\n";
	o << "/// \\ingroup std.core\n";
	o << "/// \\important THIS FILE IS AUTOMATICALLY GENERATED (see `nsl-core-generator.cpp`)\n";
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << "/// \\brief   " << (issigned ? "Signed" : "Unsigned");
	o << " integer with width of exactly " << bits << " bits\n";
	o << "/// \\ingroup std.core\n";
	o << "public class " << suffix << '\n';
	o << "{\n";
	o << "\toperator new;\n";
	craftOperator([&](uint32_t b, char targetsign)
	{
		for ( ; b >= 8; b /= 2)
		{
			o << "\toperator new (self pod: __" << targetsign << b << ");\n";
			o << "\toperator new (self cref pod: " << targetsign << b << ");\n";
		}
	});

	o << '\n';
	o << "\toperator ++self: ref " << suffix << '\n';
	o << "\t{\n";
	o << "\t\tpod = !!inc(pod);\n";
	o << "\t\treturn self;\n";
	o << "\t}\n";
	o << '\n';
	o << "\toperator self++: ref " << suffix << '\n';
	o << "\t{\n";
	o << "\t\tvar tmp = self;\n";
	o << "\t\tpod = !!inc(pod);\n";
	o << "\t\treturn tmp;\n";
	o << "\t}\n";
	o << '\n';
	o << "\toperator --self: ref " << suffix << '\n';
	o << "\t{\n";
	o << "\t\tpod = !!dec(pod);\n";
	o << "\t\treturn self;\n";
	o << "\t}\n";
	o << '\n';
	o << "\toperator self--: ref " << suffix << '\n';
	o << "\t{\n";
	o << "\t\tvar tmp = self;\n";
	o << "\t\tpod = !!dec(pod);\n";
	o << "\t\treturn tmp;\n";
	o << "\t}\n";
	o << '\n';
	o << '\n';


	auto craftMemberOperator = [&](const AnyString& op, const AnyString& intrinsic, bool prefix)
	{
		AnyString pr = (prefix and issigned) ? "i" : "";
		craftOperator([&](uint32_t b, char targetsign)
		{
			for ( ; b >= 8; b /= 2)
			{
				o << "\toperator " << op << " (cref x: " << targetsign << b << "): ref " << suffix << '\n';
				o << "\t{\n";
				o << "\t\tpod = !!" << pr << intrinsic << "(pod, x.pod);\n";
				o << "\t\treturn self;\n";
				o << "\t}\n\n";

				o << "\toperator " << op << " (x: __" << targetsign << b << "): ref " << suffix << '\n';
				o << "\t{\n";
				o << "\t\tpod = !!" << pr << intrinsic << "(pod, x);\n";
				o << "\t\treturn self;\n";
				o << "\t}\n\n";
			}
		});
	};

	craftMemberOperator("+=", "add", false);
	craftMemberOperator("-=", "sub", false);
	craftMemberOperator("*=", "mul", true);
	craftMemberOperator("/=", "div", true);

	o << "private:\n";
	o << "\t//! The real integer representation\n";
	o << "\tvar pod: __" << suffix << " = 0__" << suffix << ";\n";
	o << "}\n";
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';


	auto genGlobalCompareOperator = [&](AnyString op, AnyString builtin, char sign, uint32_t b, AnyString prefixA, AnyString prefixB)
	{
		o << "[[builtinalias: " << builtin << "]] public operator ";
		o << op << " (a: " << prefixA << suffix << ", b: " << prefixB << sign << b << "): ";
		o << ((prefixA.first() != '_' or prefixB.first() != '_') ? "ref " : "__");
		o << "bool;\n";
	};
	craft(">",  "gt",  genGlobalCompareOperator);
	craft(">=", "gte", genGlobalCompareOperator);
	craft("<",  "lt",  genGlobalCompareOperator);
	craft("<=", "lte", genGlobalCompareOperator);

	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';

	craft("==",  "eq",  genGlobalCompareOperator);
	craft("!=",  "neq", genGlobalCompareOperator);

	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';

	auto genGlobalOperator = [&](AnyString op, AnyString builtin, char sign, uint32_t b, AnyString prefixA, AnyString prefixB)
	{
		o << "[[builtinalias: " << builtin << "]] public operator ";
		o << op << " (a: " << prefixA << suffix << ", b: " << prefixB << sign << b << "): ";
		o << ((prefixA.first() != '_' or prefixB.first() != '_') ? "ref " : "__");
		o << suffix << ";\n";
	};
	craft("+", "add", genGlobalOperator);
	craft("-", "sub", genGlobalOperator);
	if (issigned)
	{
		craft("/", "idiv", genGlobalOperator);
		craft("*", "imul", genGlobalOperator);
	}
	else
	{
		craft("/", "div", genGlobalOperator);
		craft("*", "mul", genGlobalOperator);
	}

	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';

	craft("and", "and", genGlobalOperator);
	craft("or", "or", genGlobalOperator);
	craft("xor", "xor", genGlobalOperator);

	o << '\n';
	o << '\n';
	o << '\n';
	o << '\n';
	o << "// -*- mode: nany;-*-\n";
	o << "// vim: set filetype=nany:";


	IO::File::SetContent(filename, o);
}





int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "usage: <license-header-file> <folder>\n";
		return 1;
	}

	String license;
	if (IO::errNone != IO::File::LoadFromFile(license, argv[1]))
	{
		std::cerr << "failed to load header file from '" << argv[1] << "'\n";
		return 1;
	}

	AnyString folder = argv[2];

	Clob out;
	out.reserve(1024 * 32);
	String filename;
	filename.reserve(1024);

	for (uint32_t bits = 64; bits >= 8; bits /= 2)
	{
		filename.clear() << folder << "/u" << bits << ".ny";
		craftClassInt(out, bits, false, license, filename);
		filename.clear() << folder << "/i" << bits << ".ny";
		craftClassInt(out, bits, true,  license, filename);
	}
	return 0;
}
