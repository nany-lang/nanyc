#pragma once
#include <yuni/yuni.h>

#ifdef YUNI_OS_MSVC
#pragma warning(disable: 4251)
#endif


/*!
** \brief Prefix for named function operators (new, renew, copy...)
*/
#define NANY_OPERATOR_NAME_PREFIX "@"


#define likely(X)    YUNI_LIKELY(X)
#define unlikely(X)  YUNI_UNLIKELY(X)




struct nytcontext_t;



namespace Nany
{

	//! Local var ID
	typedef yuint32 LVID;

	static inline bool  lvidIsAny(LVID lvid) { return lvid == (LVID) -1; }


	class CTarget;
	class Node;
	class Atom;
	class ClassdefTable;
	class ClassdefTableView;

	class Intrinsic;


} // namespace Nany



namespace Nany
{
namespace Logs
{

	class Report;
	class Message;


} // namespace Logs
} // namespace Nany
