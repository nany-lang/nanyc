#include "thread-context.h"
#include <yuni/core/system/environment.h>
#include <yuni/core/system/windows.hdr.h>

using namespace Yuni;




namespace Nany
{
namespace VM
{


	ThreadContext::ThreadContext(Program& program, const AnyString& name)
		: program(program)
		, cf(program.cf)
		, name(name)
	{
		// default current working directory
		io.cwd = "/home";
		// fallback adapter, which nearly always returns an error
		initFallbackAdapter(io.fallbackAdapter);
	}


	ThreadContext::ThreadContext(ThreadContext& rhs)
		: program(rhs.program)
		, cf(rhs.cf)
	{
		memcpy(&io.fallbackAdapter, &rhs.io.fallbackAdapter, sizeof(nyio_adapter_t));
		io.cwd = rhs.io.cwd;

		io.mountpoints.resize(rhs.io.mountpoints.size());
		for (size_t i = 0; i != rhs.io.mountpoints.size(); ++i)
		{
			auto& original = rhs.io.mountpoints[i];
			auto& mountpoint = io.mountpoints[i];

			mountpoint.path = original.path;
			if (original.adapter.clone)
				original.adapter.clone(&original.adapter, &mountpoint.adapter);
			else
				memcpy(&mountpoint.adapter, &original.adapter, sizeof(nyio_adapter_t));
		}
	}


	ThreadContext::~ThreadContext()
	{
		for (auto& mountpoint: io.mountpoints)
		{
			if (mountpoint.adapter.release)
				mountpoint.adapter.release(&mountpoint.adapter);
		}
	}


	void ThreadContext::cerrException(const AnyString& msg)
	{
		cerr("\n\n");
		cerrColor(nyc_red);
		cerr("exception: ");
		cerrColor(nyc_white);
		cerr(msg);
		cerrColor(nyc_none);
		cerr("\n");
	}


	void ThreadContext::cerrUnknownPointer(void* ptr, uint32_t offset)
	{
		ShortString128 msg; // avoid memory allocation
		msg << "unknown pointer " << ptr << ", opcode: +";
		msg << offset;
		cerrException(msg);
	}


	void ThreadContext::IO::addMountpoint(const AnyString& path, nyio_adapter_t& adapter)
	{
		mountpoints.emplace_back();
		auto& mp = mountpoints.back();
		mp.path << path;
		mp.path.trimRight('/');
		if (mp.path.empty())
			mp.path << '/';
		memcpy(&mp.adapter, &adapter, sizeof(nyio_adapter_t));
	}


	nyio_adapter_t& ThreadContext::IO::resolve(AnyString& adapterpath, const AnyString& path)
	{
		for (auto& mountpoint: mountpoints)
		{
			const auto& mpath = mountpoint.path;
			if (path.startsWith(mpath))
			{
				uint32_t size = mpath.size();
				if (path.size() > size)
				{
					if (path[size] == '/')
					{
						adapterpath.adapt(path.c_str() + size, path.size() - size);
						return mountpoint.adapter;
					}
				}
				else
				{
					assert(adapterpath.empty());
					adapterpath = "/";
					return mountpoint.adapter;
				}
			}
		}
		adapterpath = path;
		return fallbackAdapter;
	}


	bool ThreadContext::initializeProgramSettings()
	{
		// mount '/' -> '/root'
		{
			nyio_adapter_t adapter;
			nyio_adapter_create_from_local_folder(&adapter, &cf.allocator, nullptr, 0u);
			io.addMountpoint("/root", adapter);
		}

		// mount home folder
		{
			nyio_adapter_t adapter;
			String homepath;
			if (not System::Environment::Read((System::windows ? "USERPROFILE" : "HOME"), homepath))
				return false;
			nyio_adapter_create_from_local_folder(&adapter, &cf.allocator, homepath.c_str(), homepath.size());
			io.addMountpoint("/home", adapter);
		}
		return true;
	}




} // namespace VM
} // namespace Nany
