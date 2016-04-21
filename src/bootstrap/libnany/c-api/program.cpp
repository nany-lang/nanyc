#include <yuni/yuni.h>
#include "nany/nany.h"

using namespace Yuni;





extern "C" void nany_program_cf_reset(nyprogram_cf_t* cf)
{
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nyprogram_cf_t));
	nany_memalloc_set_default(&(cf->allocator));
	nany_console_cf_set_stdcout(&cf->console);
}
