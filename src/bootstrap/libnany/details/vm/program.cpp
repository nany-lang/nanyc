#include "program.h"
#include "details/atom/atom.h"
#include "details/intrinsic/intrinsic-table.h"
#include "types.h"
#include "vm.h"

using namespace Yuni;





namespace Nany
{
namespace VM
{


	void Program::destroy()
	{
		this->~Program();

		auto& allocator = const_cast<nyallocator_t&>(cf.allocator);
		allocator.deallocate(&allocator, this, sizeof(Nany::VM::Program));
	}



	int Program::execute(int argc, const char** argv)
	{
		// TODO Take input arguments into consideration
		(void) argc;
		(void) argv;

		retvalue = 0;
		bool success = false;
		uint32_t atomid = Nany::ref(build).main.atomid;
		uint32_t instanceid = Nany::ref(build).main.instanceid;

		try
		{
			auto& sequence = map.sequence(atomid, instanceid);
			ThreadContext thrctx{*this, "main"};

			retvalue = static_cast<int>(thrctx.invoke(sequence, atomid, instanceid));
			success = true;
		}
		catch (const CodeAbort&)
		{
			// error already handled
		}
		catch (const std::bad_alloc&)
		{
			// already reported by the custom memory allocator
		}
		catch (const std::exception& e)
		{
			printStderr("error: exception: ");
			printStderr(e.what());
			printStderr("\n");
		}
		catch (...)
		{
			printStderr("error: exception received: aborting\n");
		}

		// always flush to make sure that the listener will update the output
		// (especially useful when embedded into a C/C++ application)
		if (cf.console.flush)
		{
			cf.console.flush(cf.console.internal, nycerr);
			cf.console.flush(cf.console.internal, nycout);
		}
		return success ? retvalue : 1;
	}




} // namespace VM
} // namespace Nany
