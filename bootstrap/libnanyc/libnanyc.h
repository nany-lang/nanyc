#pragma once
#include <yuni/yuni.h>

#ifdef YUNI_OS_MSVC
#pragma warning(disable: 4251)
#endif

#define likely(X)    YUNI_LIKELY(X)
#define unlikely(X)  YUNI_UNLIKELY(X)







namespace ny
{

	class CTarget;
	struct Atom;
	class ClassdefTable;
	class ClassdefTableView;
	class Intrinsic;



} // namespace ny


namespace ny { namespace AST { class Node; } }


namespace ny
{
namespace Logs
{

	class Report;
	class Message;


} // namespace Logs
} // namespace ny
