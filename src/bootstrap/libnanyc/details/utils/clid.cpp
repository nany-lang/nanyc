#include "clid.h"
#include <iostream>

using namespace Yuni;




std::ostream& operator << (std::ostream& out, const Nany::CLID& rhs)
{
	out << '{' << rhs.atomid() << ':' << rhs.lvid() << '}';
	return out;
}
