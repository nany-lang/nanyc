#include "isolate.h"
#include "details/ir/program.h"
#include "details/reporting/report.h"
#include "details/atom/classdef-table-view.h"
#include "details/vm/vm.h"

using namespace Yuni;




namespace Nany
{

	Isolate::AttachedProgramRef::~AttachedProgramRef()
	{
		if (owned)
			delete program;
	}


	void Isolate::print(Logs::Report& report) const
	{
		Clob out;
		out.reserve(1024);
		uint index = 0;

		for (auto& ref: pAttachedPrograms)
		{
			if (ref.program != nullptr)
			{
				auto trace = (report.trace() << "\n");
				trace.data().prefix << "SOURCE COMPILATION PROGRAM " << index;
				ref.program->print(trace.data().message);
				trace.data().message.shrink();
				report.info(); // for beauty
			}
			else
				assert(false);
			++index;
		}
	}


	int Isolate::run(bool& success, nycontext_t& ctx, const AnyString& entrypoint)
	{
		success = false;

		const IR::Program* program;
		{
			// try to find the entrypoint
			Atom* entrypointAtom = nullptr;
			{
				bool canContinue = true;
				classdefTable.atoms.root.eachChild(entrypoint, [&](Atom& child) -> bool
				{
					if (entrypointAtom != nullptr)
					{
						canContinue = false;
						String msg;
						msg << "error: failed to instanciate '" << entrypoint << "': multiple entry points found\n";
						ctx.console.write_stderr(&ctx, msg.c_str(), msg.size());
						ctx.console.flush_stderr(&ctx);
						return false;
					}
					entrypointAtom = &child;
					return true;
				});

				if (not canContinue)
					return 0;
			}

			if (unlikely(nullptr == entrypointAtom))
			{
				String msg;
				msg << "error: failed to instanciate '" << entrypoint << "()': function not found\n";
				ctx.console.write_stderr(&ctx, msg.c_str(), msg.size());
				ctx.console.flush_stderr(&ctx);
				return 0;
			}

			program = entrypointAtom->fetchInstance(0);
			if (unlikely(nullptr == program))
			{
				String msg;
				msg << "error: failed to instanciate '" << entrypoint << "()': function not found\n";
				ctx.console.write_stderr(&ctx, msg.c_str(), msg.size());
				ctx.console.flush_stderr(&ctx);
				return 0;
			}
		}
		return VM::execute(success, ctx, *program, classdefTable.atoms);
	}



} // namespace Nany
