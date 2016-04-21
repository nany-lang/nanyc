#include "nany/nany.h"
#include "details/fwd.h"
#include <yuni/core/system/console.h>
#include "details/grammar/nany.h"
#include <stdio.h>

#ifdef YUNI_OS_MSVC
#include <io.h>
#define STDOUT_FILENO  1
#define STDERR_FILENO  2
#else
#include <unistd.h>
#endif

using namespace Yuni;





namespace // anonymous
{

	template<bool FromFileT>
	static inline bool nany_print_ast(const AnyString& text, int fd, bool unixcolors)
	{
		if (unlikely(fd < 0))
			return false;

		if (unlikely(text.empty()))
			return false;

		Nany::AST::Parser parser;
		if (not (FromFileT ? parser.loadFromFile(text) : parser.load(text)))
			return false;

		if (nullptr == parser.root)
			return false;

		// invalidate unix colors if not available
		bool hasUnixColors = (unixcolors)
			and ((fd == STDOUT_FILENO and System::Console::IsStdoutTTY())
			or   (fd == STDERR_FILENO and System::Console::IsStderrTTY()));

		Clob out;
		Nany::AST::Node::Export(out, *parser.root, hasUnixColors);

		#ifdef YUNI_OS_MSVC
		int w = _write(fd, out.c_str(), static_cast<unsigned int>(out.size()));
		return (w == static_cast<int>(out.size()));
		#else
		sint64 w = ::write(fd, out.c_str(), out.size());
		return (w == static_cast<sint64>(out.size()));
		#endif
	}


} // anonymous namespace





extern "C" nybool_t nany_print_ast_from_file_n(const char* filename, size_t length, int fd, nybool_t unixcolors)
{
	try
	{
		bool colors = (unixcolors == nytrue);
		bool ok = nany_print_ast<true>(AnyString{filename, (uint32_t) length}, fd, colors);
		return ok ? nytrue : nyfalse;
	}
	catch (...) {}
	return nyfalse;
}


extern "C" nybool_t nany_print_ast_from_memory_n(const char* content, size_t length, int fd, nybool_t unixcolors)
{
	try
	{
		bool colors = (unixcolors == nytrue);
		bool ok = nany_print_ast<false>(AnyString{content, (uint32_t) length}, fd, colors);
		return ok ? nytrue : nyfalse;
	}
	catch (...) {}
	return nyfalse;
}
