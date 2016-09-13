#pragma once
#include <yuni/yuni.h>

#ifdef YUNI_OS_MSVC
#pragma warning(disable: 4251)
#endif

#define likely(X)    YUNI_LIKELY(X)
#define unlikely(X)  YUNI_UNLIKELY(X)







namespace Nany
{

	//! Local var ID
	using LVID = uint32_t;

	static inline constexpr bool lvidIsAny(LVID lvid) { return lvid == (LVID) -1; }


	class CTarget;
	class Atom;
	class ClassdefTable;
	class ClassdefTableView;
	class Intrinsic;



} // namespace Nany


namespace Nany { namespace AST { class Node; } }


namespace Nany
{
namespace Logs
{

	class Report;
	class Message;


} // namespace Logs
} // namespace Nany
